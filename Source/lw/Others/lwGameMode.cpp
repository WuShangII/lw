// Copyright Epic Games, Inc. All Rights Reserved.

#include "lwGameMode.h"
#include "Enemy/EnemySpawner.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Player/lwPlayerController.h"
#include "TimerManager.h"
#include "lw.h"

AlwGameMode::AlwGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AlwGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (MatchState == ESurvivalMatchState::VictorySlowMotion
		&& GetWorld()->GetRealTimeSeconds() - VictorySlowMotionStartTime >= VictorySlowMotionDuration)
	{
		FinishVictorySlowMotion();
	}
}

void AlwGameMode::StartPlay()
{
	Super::StartPlay();
	for (TActorIterator<AEnemySpawner> It(GetWorld()); It; ++It)
	{
		AEnemySpawner* Spawner = *It;
		EnemySpawners.Add(Spawner);
		Spawner->OnEnemyCountChanged.AddUObject(this, &AlwGameMode::CheckVictory);
	}
	MatchStartTime = GetWorld()->GetTimeSeconds();
	GetWorldTimerManager().SetTimer(SpawnEndTimerHandle, this,
		&AlwGameMode::FinishSpawning, FMath::Max(SpawnDuration, 0.1f), false);
}

void AlwGameMode::FinishSpawning()
{
	for (const TWeakObjectPtr<AEnemySpawner>& Spawner : EnemySpawners)
	{
		if (Spawner.IsValid())
		{
			Spawner->FinishSpawning();
		}
	}
	MatchState = ESurvivalMatchState::WaitingForClear;
	UE_LOG(Loglw, Log, TEXT("Spawning finished. Remaining enemies: %d"), GetTotalAliveEnemyCount());
	CheckVictory();
}

int32 AlwGameMode::GetTotalAliveEnemyCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AEnemySpawner>& Spawner : EnemySpawners)
	{
		if (Spawner.IsValid())
		{
			Count += Spawner->GetAliveEnemyCount();
		}
	}
	return Count;
}

float AlwGameMode::GetElapsedMatchTime() const
{
	return (MatchState == ESurvivalMatchState::Victory || MatchState == ESurvivalMatchState::VictorySlowMotion || MatchState == ESurvivalMatchState::Defeat)
		? VictoryElapsedTime
		: static_cast<float>(GetWorld()->GetTimeSeconds() - MatchStartTime);
}

void AlwGameMode::CheckVictory()
{
	if (GetWorld()->bIsTearingDown || MatchState != ESurvivalMatchState::WaitingForClear
		|| GetTotalAliveEnemyCount() != 0)
	{
		return;
	}
	VictoryElapsedTime = GetElapsedMatchTime();
	MatchState = ESurvivalMatchState::VictorySlowMotion;
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		PlayerPawn->SetCanBeDamaged(false);
	}
	if (AlwPlayerController* PC = Cast<AlwPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->CloseUpgradeMenu(false);
	}
	VictorySlowMotionStartTime = GetWorld()->GetRealTimeSeconds();
	UGameplayStatics::SetGlobalTimeDilation(this, FMath::Clamp(VictoryTimeDilation, 0.01f, 1.f));
	SetActorTickEnabled(true);
	UE_LOG(Loglw, Log, TEXT("Victory! All spawned enemies cleared."));
	OnVictory();
}

void AlwGameMode::FinishVictorySlowMotion()
{
	MatchState = ESurvivalMatchState::Victory;
	SetActorTickEnabled(false);
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
	UGameplayStatics::SetGamePaused(this, true);
	if (AlwPlayerController* PlayerController = Cast<AlwPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PlayerController->ShowVictoryScreen();
	}
	UE_LOG(Loglw, Log, TEXT("Victory slow motion finished. Game paused."));
	OnVictoryFinished();
}

void AlwGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetActorTickEnabled(false);
	if (MatchState == ESurvivalMatchState::VictorySlowMotion)
	{
		UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
	}
	GetWorldTimerManager().ClearTimer(SpawnEndTimerHandle);
	for (const TWeakObjectPtr<AEnemySpawner>& Spawner : EnemySpawners)
	{
		if (Spawner.IsValid())
		{
			Spawner->OnEnemyCountChanged.RemoveAll(this);
		}
	}
	EnemySpawners.Reset();
	Super::EndPlay(EndPlayReason);
}

void AlwGameMode::HandlePlayerDefeated()
{
	if (MatchState != ESurvivalMatchState::Spawning && MatchState != ESurvivalMatchState::WaitingForClear) return;
	VictoryElapsedTime = GetElapsedMatchTime();
	MatchState = ESurvivalMatchState::Defeat;
	if (AlwPlayerController* PC = Cast<AlwPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->CloseUpgradeMenu(false);
	}
	GetWorldTimerManager().ClearTimer(SpawnEndTimerHandle);
	for (const TWeakObjectPtr<AEnemySpawner>& Spawner : EnemySpawners)
	{
		if (Spawner.IsValid()) Spawner->FinishSpawning();
	}
	UGameplayStatics::SetGamePaused(this, true);
	UE_LOG(Loglw, Log, TEXT("Player defeated."));
	OnDefeat();
}

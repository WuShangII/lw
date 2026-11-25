// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnemyBase.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "lw.h"
#include "TimerManager.h"

bool FEnemySpawnEntry::IsSelectable() const
{
	return EnemyClass && !EnemyClass->HasAnyClassFlags(CLASS_Abstract)
		&& FMath::IsFinite(Weight) && Weight > 0.f;
}

// Sets default values
AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SetRootComponent(SpawnArea);
	SpawnArea->SetBoxExtent(FVector(1000.f, 1000.f, 200.f));
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnArea->SetGenerateOverlapEvents(false);
}

AEnemyBase* AEnemySpawner::SpawnOneEnemy()
{
	UpdateSpawnPhase();
	return SpawnEnemyOfClass(EnemyClass);
}

AEnemyBase* AEnemySpawner::SpawnEnemyOfClass(TSubclassOf<AEnemyBase> ClassToSpawn)
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld() || !HasAuthority() || bSpawningFinished)
	{
		return nullptr;
	}

	if (!ClassToSpawn || ClassToSpawn->HasAnyClassFlags(CLASS_Abstract) || !IsValid(SpawnArea))
	{
		return nullptr;
	}

	SpawnedEnemies.RemoveAll([](const TWeakObjectPtr<AEnemyBase>& Enemy) { return !Enemy.IsValid(); });
	if (SpawnedEnemies.Num() >= FMath::Max(GetCurrentMaxAliveEnemies(), 0))
	{
		return nullptr;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!IsValid(PlayerPawn))
	{
		return nullptr;
	}

	if (!FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
	{
		return nullptr;
	}

	const FVector AreaScale = SpawnArea->GetComponentScale();
	if (FVector::DotProduct(SpawnArea->GetUpVector(), FVector::UpVector) < 0.999f
		|| AreaScale.X <= KINDA_SMALL_NUMBER || AreaScale.Y <= KINDA_SMALL_NUMBER
		|| AreaScale.Z <= KINDA_SMALL_NUMBER)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
	const FRotator SpawnRotation(0.f, SpawnArea->GetComponentRotation().Yaw, 0.f);
	const int32 AttemptLimit = FMath::Clamp(MaxSpawnAttempts, 1, 100);
	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	const float MinSpawnDistanceSquared = FMath::Square(MinSpawnDistance);

	for (int32 Attempt = 0; Attempt < AttemptLimit; ++Attempt)
	{
		FVector SpawnLocation;
		if (!FindSpawnLocation(ClassToSpawn, SpawnLocation))
		{
			continue;
		}

		if (FVector::DistSquared2D(SpawnLocation, PlayerLocation) < MinSpawnDistanceSquared)
		{
			continue;
		}

		AEnemyBase* Enemy = World->SpawnActor<AEnemyBase>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParameters);
		if (IsValid(Enemy))
		{
			SpawnedEnemies.Add(Enemy);
			Enemy->OnDestroyed.AddDynamic(this, &AEnemySpawner::OnSpawnedEnemyDestroyed);
			OnEnemyCountChanged.Broadcast();
			return Enemy;
		}
	}

	return nullptr;
}

TSubclassOf<AEnemyBase> AEnemySpawner::SelectAutoEnemyClass() const
{
	if (!SpawnPhases.IsValidIndex(CurrentPhaseIndex))
	{
		return nullptr;
	}
	const TArray<FEnemySpawnEntry>& Entries = SpawnPhases[CurrentPhaseIndex].Entries;
	double TotalWeight = 0.0;
	for (const FEnemySpawnEntry& Entry : Entries)
	{
		if (Entry.IsSelectable())
		{
			TotalWeight += Entry.Weight;
		}
	}
	if (TotalWeight <= 0.0)
	{
		return nullptr;
	}
	double RemainingWeight = FMath::FRand() * TotalWeight;
	TSubclassOf<AEnemyBase> LastValidClass;
	for (const FEnemySpawnEntry& Entry : Entries)
	{
		if (!Entry.IsSelectable())
		{
			continue;
		}
		LastValidClass = Entry.EnemyClass;
		if (RemainingWeight < Entry.Weight)
		{
			return Entry.EnemyClass;
		}
		RemainingWeight -= Entry.Weight;
	}
	return LastValidClass;
}

bool AEnemySpawner::FindSpawnLocation(TSubclassOf<AEnemyBase> ClassToSpawn, FVector& OutLocation) const
{
	const AEnemyBase* EnemyDefaults = ClassToSpawn.GetDefaultObject();
	UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!Navigation || !EnemyDefaults || !EnemyDefaults->GetCapsuleComponent() || !IsValid(SpawnArea))
	{
		return false;
	}

	const FVector Extent = SpawnArea->GetUnscaledBoxExtent();
	const FTransform AreaTransform = SpawnArea->GetComponentTransform();
	const FVector LocalCandidate(FMath::FRandRange(-Extent.X, Extent.X), FMath::FRandRange(-Extent.Y, Extent.Y), 0.f);
	const FVector WorldCandidate = AreaTransform.TransformPosition(LocalCandidate);
	const FVector QueryExtent = NavigationQueryExtent.GetAbs().ComponentMax(FVector(1.f));
	FNavLocation GroundLocation;
	const FNavAgentProperties& AgentProperties = EnemyDefaults->GetCharacterMovement()->GetNavAgentPropertiesRef();
	if (!Navigation->ProjectPointToNavigation(WorldCandidate, GroundLocation, QueryExtent, &AgentProperties))
	{
		return false;
	}

	const FVector LocalGround = AreaTransform.InverseTransformPosition(GroundLocation.Location);
	if (FMath::Abs(LocalGround.X) > Extent.X || FMath::Abs(LocalGround.Y) > Extent.Y
		|| FMath::Abs(LocalGround.Z) > Extent.Z)
	{
		return false;
	}

	const float CapsuleHalfHeight = EnemyDefaults->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	OutLocation = GroundLocation.Location + FVector::UpVector * (CapsuleHalfHeight + FMath::Max(0.f, SpawnHeightOffset));
	return true;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	if (bAutoSpawnEnabled)
	{
		StartSpawning();
	}
}

void AEnemySpawner::StartSpawning()
{
	if (!HasAuthority() || !HasActorBegunPlay() || bSpawningFinished)
	{
		return;
	}
	bAutoSpawnEnabled = true;
	if (bSpawnClockRunning)
	{
		return;
	}
	SpawnClockStart = GetWorld()->GetTimeSeconds();
    bSpawnClockRunning = true;
    UpdateSpawnPhase();
    GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AEnemySpawner::UpdateSpawnPhase, 0.1f, true);
    if (SpawnPhases.IsValidIndex(CurrentPhaseIndex))
    {
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnBatch,
            FMath::Max(SpawnPhases[CurrentPhaseIndex].SpawnInterval, 0.1f), true,
            FMath::Max(InitialSpawnDelay, 0.f));
    }
}

void AEnemySpawner::StopSpawning()
{
	if (bSpawnClockRunning)
    {
        AccumulatedSpawnTime += GetWorld()->GetTimeSeconds() - SpawnClockStart;
        bSpawnClockRunning = false;
    }
    bAutoSpawnEnabled = false;
    GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
    GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}

void AEnemySpawner::FinishSpawning()
{
	bSpawningFinished = true;
	StopSpawning();
}

void AEnemySpawner::SetAutoSpawnEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		StartSpawning();
	}
	else
	{
		StopSpawning();
	}
}

int32 AEnemySpawner::GetAliveEnemyCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AEnemyBase>& Enemy : SpawnedEnemies)
	{
		Count += Enemy.IsValid() ? 1 : 0;
	}
	return Count;
}

void AEnemySpawner::SpawnBatch()
{
	if (!bAutoSpawnEnabled || bSpawningFinished)
	{
		StopSpawning();
		return;
	}
	const int32 PreviousPhaseIndex = CurrentPhaseIndex;
	UpdateSpawnPhase();
	if (!SpawnPhases.IsValidIndex(CurrentPhaseIndex) || PreviousPhaseIndex != CurrentPhaseIndex)
	{
		return;
	}
    const int32 AvailableSlots = FMath::Max(0, GetCurrentMaxAliveEnemies() - GetAliveEnemyCount());
	const int32 Count = FMath::Min(FMath::Clamp(SpawnPhases[CurrentPhaseIndex].EnemiesPerBatch, 1, 100), AvailableSlots);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const TSubclassOf<AEnemyBase> SelectedClass = SelectAutoEnemyClass();
		if (!SelectedClass)
		{
			break;
		}
		SpawnEnemyOfClass(SelectedClass);
	}
}

void AEnemySpawner::OnSpawnedEnemyDestroyed(AActor* DestroyedActor)
{
	SpawnedEnemies.RemoveAll([DestroyedActor](const TWeakObjectPtr<AEnemyBase>& Enemy)
	{
		return !Enemy.IsValid() || Enemy.Get() == DestroyedActor;
	});
	OnEnemyCountChanged.Broadcast();
}

void AEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSpawning();
	for (const TWeakObjectPtr<AEnemyBase>& Enemy : SpawnedEnemies)
	{
		if (Enemy.IsValid())
		{
			Enemy->OnDestroyed.RemoveDynamic(this, &AEnemySpawner::OnSpawnedEnemyDestroyed);
		}
	}
	SpawnedEnemies.Reset();
	OnEnemyCountChanged.Clear();
	Super::EndPlay(EndPlayReason);

}


float AEnemySpawner::GetSpawningElapsedTime() const
{
    return static_cast<float>(AccumulatedSpawnTime + (bSpawnClockRunning
        ? GetWorld()->GetTimeSeconds() - SpawnClockStart : 0.0));
}

int32 AEnemySpawner::GetCurrentMaxAliveEnemies() const
{
    return SpawnPhases.IsValidIndex(CurrentPhaseIndex)
        ? SpawnPhases[CurrentPhaseIndex].MaxAliveEnemies : ManualSpawnLimit;
}

void AEnemySpawner::UpdateSpawnPhase()
{
    const float Elapsed = GetSpawningElapsedTime();
    int32 NewIndex = INDEX_NONE;
    float LatestStart = -1.f;
    for (int32 Index = 0; Index < SpawnPhases.Num(); ++Index)
    {
        const float Start = SpawnPhases[Index].StartTime;
        if (Start >= 0.f && Start <= Elapsed && Start >= LatestStart)
        {
            LatestStart = Start;
            NewIndex = Index;
        }
    }
    if (NewIndex != CurrentPhaseIndex)
    {
        CurrentPhaseIndex = NewIndex;
        if (bSpawnClockRunning)
        {
            GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
            if (SpawnPhases.IsValidIndex(CurrentPhaseIndex))
            {
                GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnBatch,
                    FMath::Max(SpawnPhases[CurrentPhaseIndex].SpawnInterval, 0.1f), true);
            }
        }
    }
}

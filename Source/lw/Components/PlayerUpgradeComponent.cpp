#include "PlayerUpgradeComponent.h"
#include "Others/UpgradeDefinition.h"
#include "Components/PlayerStatsComponent.h"
#include "lw.h"
#include "Others/lwGameMode.h"
#include "Components/RangedAttackComponent.h"
#include "Player/lwCharacter.h"
#include "Player/lwPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"

UPlayerUpgradeComponent::UPlayerUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>())
	{
		Stats->OnLevelsGained.AddDynamic(this, &UPlayerUpgradeComponent::HandleLevelsGained);
	}
}

void UPlayerUpgradeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>())
	{
		Stats->OnLevelsGained.RemoveDynamic(this, &UPlayerUpgradeComponent::HandleLevelsGained);
	}
	GetWorld()->GetTimerManager().ClearTimer(ChoiceTimer);
	Super::EndPlay(EndPlayReason);
}

int32 UPlayerUpgradeComponent::GetUpgradeStacks(FName UpgradeId) const
{
	return SelectedStacks.FindRef(UpgradeId);
}

void UPlayerUpgradeComponent::HandleLevelsGained(int32 LevelsGained)
{
	PendingChoices += LevelsGained;
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (!TimerManager.TimerExists(ChoiceTimer))
	{
		ChoiceTimer = TimerManager.SetTimerForNextTick(this, &UPlayerUpgradeComponent::ProcessPendingChoices);
	}
	UE_LOG(Loglw, Log, TEXT("Upgrade pending choices: %d"), PendingChoices);
}

void UPlayerUpgradeComponent::GenerateChoices()
{
	TArray<UUpgradeDefinition*> Candidates;
	TSet<FName> SeenIds;
	for (UUpgradeDefinition* Upgrade : UpgradePool)
	{
		if (!IsValid(Upgrade) || Upgrade->UpgradeId.IsNone() || SeenIds.Contains(Upgrade->UpgradeId)
			|| !FMath::IsFinite(Upgrade->Weight) || Upgrade->Weight <= 0.f
			|| !Upgrade->HasValidEffects() || GetUpgradeStacks(Upgrade->UpgradeId) >= Upgrade->MaxStacks) continue;
		SeenIds.Add(Upgrade->UpgradeId);
		Candidates.Add(Upgrade);
	}
	++ChoiceRevision;
	CurrentChoices.Reset();
	while (CurrentChoices.Num() < 3 && !Candidates.IsEmpty())
	{
		double TotalWeight = 0.0;
		for (const UUpgradeDefinition* Upgrade : Candidates) TotalWeight += Upgrade->Weight;
		double Roll = FMath::FRand() * TotalWeight;
		int32 SelectedIndex = Candidates.Num() - 1;
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			Roll -= Candidates[Index]->Weight;
			if (Roll < 0.0)
			{
				SelectedIndex = Index;
				break;
			}
		}
		CurrentChoices.Add(Candidates[SelectedIndex]);
		UE_LOG(Loglw, Log, TEXT("Upgrade candidate: %s"), *Candidates[SelectedIndex]->UpgradeId.ToString());
		Candidates.RemoveAt(SelectedIndex);
	}
	if (CurrentChoices.IsEmpty())
	{
		PendingChoices = 0;
		UE_LOG(Loglw, Warning, TEXT("No eligible upgrades. Check UpgradePool, IDs, effects and weights."));
		return;
	}
	OnChoicesReady.Broadcast();
}

void UPlayerUpgradeComponent::CancelChoices()
{
	GetWorld()->GetTimerManager().ClearTimer(ChoiceTimer);
	PendingChoices = 0;
	CurrentChoices.Reset();
	++ChoiceRevision;
}

void UPlayerUpgradeComponent::ProcessPendingChoices()
{
	GetWorld()->GetTimerManager().ClearTimer(ChoiceTimer);
	const AlwGameMode* Mode = GetWorld()->GetAuthGameMode<AlwGameMode>();
	if (!Mode || !Mode->CanPauseMatch())
	{
		CancelChoices();
		return;
	}
	if (PendingChoices <= 0) return;
	if (CurrentChoices.IsEmpty()) GenerateChoices();
	APawn* Pawn = Cast<APawn>(GetOwner());
	AlwPlayerController* PC = Pawn ? Cast<AlwPlayerController>(Pawn->GetController()) : nullptr;
	if (PC && !CurrentChoices.IsEmpty()) PC->ShowUpgradeMenu(this);
}

bool UPlayerUpgradeComponent::ChooseUpgrade(UUpgradeDefinition* Upgrade, int32 Revision)
{
	const AlwGameMode* Mode = GetWorld()->GetAuthGameMode<AlwGameMode>();
	URangedAttackComponent* Ranged = GetOwner()->FindComponentByClass<URangedAttackComponent>();
	if (!Mode || !Mode->CanPauseMatch() || !Ranged || Revision != ChoiceRevision || PendingChoices <= 0
		|| !IsValid(Upgrade) || !CurrentChoices.Contains(Upgrade)
		|| GetUpgradeStacks(Upgrade->UpgradeId) >= Upgrade->MaxStacks) return false;
	if (!Upgrade->HasValidEffects()) return false;
	AlwCharacter* PlayerCharacter = Cast<AlwCharacter>(GetOwner());
	UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>();
	if (!PlayerCharacter || !Stats || !Stats->IsAlive()) return false;
	++ChoiceRevision;
	CurrentChoices.Reset();
	for (const FUpgradeEffect& Effect : Upgrade->Effects)
	{
		switch (Effect.Type)
		{
		case EUpgradeEffectType::AttackSpeed: Ranged->AddAttackSpeedMultiplier(Effect.Amount); break;
		case EUpgradeEffectType::ProjectileDamage: Ranged->AddProjectileDamageMultiplier(Effect.Amount); break;
		case EUpgradeEffectType::ProjectileLifetime: Ranged->AddProjectileLifetimeBonus(Effect.Amount); break;
		case EUpgradeEffectType::ProjectileCount: Ranged->AddProjectileCount(static_cast<int32>(Effect.Amount)); break;
		case EUpgradeEffectType::PierceCount: Ranged->AddPierceCount(static_cast<int32>(Effect.Amount)); break;
		case EUpgradeEffectType::SpreadAngle: Ranged->AddSpreadAngle(Effect.Amount); break;
		case EUpgradeEffectType::Burn: Ranged->AddBurnDamage(Effect.Amount, Effect.BurnDuration, Effect.BurnPeriod); break;
		case EUpgradeEffectType::MeleeDamage: PlayerCharacter->AddMeleeDamageMultiplier(Effect.Amount); break;
		case EUpgradeEffectType::MeleeSpeed: PlayerCharacter->AddMeleeSpeedMultiplier(Effect.Amount); break;
		case EUpgradeEffectType::MaxHealth: Stats->IncreaseMaxHealth(Effect.Amount); break;
        case EUpgradeEffectType::Frost: Ranged->AddFrostStrength(Effect.Amount, Effect.FrostDuration); break;
        case EUpgradeEffectType::Knockback: PlayerCharacter->AddMeleeKnockback(Effect.Amount); break;
        case EUpgradeEffectType::HealOnKill: Stats->AddHealOnKill(Effect.Amount); break;
		case EUpgradeEffectType::MoveSpeed: PlayerCharacter->AddMoveSpeedMultiplier(Effect.Amount); break;
		}
	}
	++SelectedStacks.FindOrAdd(Upgrade->UpgradeId);
	--PendingChoices;
	if (PendingChoices > 0) GenerateChoices();
	return true;
}

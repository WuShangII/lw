// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStatsComponent.h"

UPlayerStatsComponent::UPlayerStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerStatsComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	CurrentMana = MaxMana;
}

void UPlayerStatsComponent::AddExperience(float Amount)
{
	if (Amount <= 0.f || !FMath::IsFinite(Amount))
	{
		return;
	}

	MaxExperience = FMath::Max(MaxExperience, 1.f);
	int32 LevelsGained = 0;
	CurrentExperience += Amount;
	while (CurrentExperience >= MaxExperience)
	{
		CurrentExperience -= MaxExperience;
		++Level;
		++LevelsGained;
		MaxExperience += FMath::Max(ExperienceRequirementIncrease, 0.f);
	}
	if (LevelsGained > 0) OnLevelsGained.Broadcast(LevelsGained);
}

float UPlayerStatsComponent::ApplyHealthDamage(float Amount)
{
	if (!FMath::IsFinite(Amount) || Amount <= 0.f || CurrentHealth <= 0.f) return 0.f;
	const float AppliedDamage = FMath::Min(Amount, CurrentHealth);
	CurrentHealth -= AppliedDamage;
	return AppliedDamage;
}

void UPlayerStatsComponent::IncreaseMaxHealth(float Amount)
{
	MaxHealth += Amount;
	CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);
}

void UPlayerStatsComponent::AddHealOnKill(float Amount)
{
    HealOnKill += Amount;
}

void UPlayerStatsComponent::RestoreHealthOnKill()
{
    if (IsAlive()) CurrentHealth = FMath::Min(CurrentHealth + HealOnKill, MaxHealth);
}

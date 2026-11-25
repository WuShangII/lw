// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatsComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLevelsGained, int32, LevelsGained);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LW_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="Stats|Experience")
	FOnPlayerLevelsGained OnLevelsGained;

	// Sets default values for this component's properties
	UPlayerStatsComponent();
	float ApplyHealthDamage(float Amount);
	void IncreaseMaxHealth(float Amount);
	void AddHealOnKill(float Amount);
	void RestoreHealthOnKill();
	bool IsAlive() const { return CurrentHealth > 0.f; }
	float GetAttackPower() const { return AttackPower; }
	UFUNCTION(BlueprintCallable, Category = "Stats|Experience")
	void AddExperience(float Amount);

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stats|Health")
	float HealOnKill = 0.f;
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats|Health")
	float CurrentHealth = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats|Experience")
	float CurrentExperience = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats|Experience")
	int32 Level = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Experience", meta = (ClampMin = "1.0"))
	float MaxExperience = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Experience", meta = (ClampMin = "0.0"))
	float ExperienceRequirementIncrease = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Combat", meta = (ClampMin = "0.0"))
	float AttackPower = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Mana", meta = (ClampMin = "0.0"))
	float MaxMana = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats|Mana")
	float CurrentMana = 0.f;

		
};

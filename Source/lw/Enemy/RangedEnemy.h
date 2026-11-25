// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "RangedEnemy.generated.h"

/**
 * 
 */
class AProjectileBase;

UCLASS()
class LW_API ARangedEnemy : public AEnemyBase
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Ranged")
	TSubclassOf<AProjectileBase> ProjectileClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Ranged", meta=(ClampMin="0.0", Units="cm"))
	float ProjectileSpawnDistance = 80.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Ranged", meta=(Units="cm"))
	float ProjectileSpawnHeightOffset = 0.f;
	virtual void ExecuteAttackHit(AActor* Target) override;
	
};

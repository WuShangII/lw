// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "MeleeEnemy.generated.h"

/**
 * 
 */
UCLASS()
class LW_API AMeleeEnemy : public AEnemyBase
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Attack", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg"))
	float AttackHalfAngle = 60.f;
	virtual void ExecuteAttackHit(AActor* Target) override;
	
};

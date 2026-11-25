#pragma once

#include "CoreMinimal.h"
#include "ProjectileLaunchData.generated.h"

class AController;

USTRUCT(BlueprintType)
struct LW_API FProjectileHitEffectSpec
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Effect")
	FName EffectId = NAME_None;
	UPROPERTY(BlueprintReadOnly, Category="Effect")
	float Magnitude = 0.f;
	UPROPERTY(BlueprintReadOnly, Category="Effect")
	float Duration = 0.f;
	UPROPERTY(BlueprintReadOnly, Category="Effect")
	float Period = 0.f;
};

USTRUCT(BlueprintType)
struct LW_API FProjectileLaunchData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Projectile")
	float Damage = 0.f;
	UPROPERTY(BlueprintReadOnly, Category="Projectile")
	bool bTargetsPlayer = false;
	UPROPERTY(BlueprintReadOnly, Category="Projectile")
	float Lifetime = 3.f;
	UPROPERTY(BlueprintReadOnly, Category="Projectile")
	float Speed = 1200.f;
	UPROPERTY(BlueprintReadOnly, Category="Projectile")
	FVector Direction = FVector::ForwardVector;
	UPROPERTY(BlueprintReadOnly, Category="Projectile")
	TWeakObjectPtr<AController> DamageInstigator;

	// 预留数据，后续阶段接入穿透和状态效果。
	UPROPERTY(BlueprintReadOnly, Category="Projectile")
	int32 PierceCount = 0;
	UPROPERTY(BlueprintReadOnly, Category="Projectile")
	TArray<FProjectileHitEffectSpec> HitEffects;
};

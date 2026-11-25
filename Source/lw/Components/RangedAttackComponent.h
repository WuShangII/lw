#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RangedAttackComponent.generated.h"

class AProjectileBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LW_API URangedAttackComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    URangedAttackComponent();
    bool Fire(const FVector& Direction);
    void CommitAttackCooldown();
    float GetAttackSpeedMultiplier() const { return FMath::Max(AttackSpeedMultiplier, 0.1f); }

    UFUNCTION(BlueprintPure, Category="Ranged|Attack")
    float GetAttackInterval() const;
    UFUNCTION(BlueprintPure, Category="Ranged|Attack")
    float GetRemainingAttackCooldown() const;
    UFUNCTION(BlueprintCallable, Category="Ranged|Upgrades")
    void AddAttackSpeedMultiplier(float Amount);

    UFUNCTION(BlueprintCallable, Category="Ranged|Upgrades")
    void AddProjectileDamageMultiplier(float Amount);
    UFUNCTION(BlueprintCallable, Category="Ranged|Upgrades")
    void AddProjectileLifetimeBonus(float Seconds);

    UFUNCTION(BlueprintCallable, Category="Ranged|Upgrades")
    void AddProjectileCount(int32 Amount);
    UFUNCTION(BlueprintCallable, Category="Ranged|Upgrades")
    void AddSpreadAngle(float Degrees);
    UFUNCTION(BlueprintCallable, Category="Ranged|Upgrades")
    void AddPierceCount(int32 Amount);
    UFUNCTION(BlueprintCallable, Category="Ranged|Effects")
    void SetBurnEffect(float DamagePerTick, float Duration, float Period);
    void AddBurnDamage(float Amount, float Duration, float Period);
    void AddFrostStrength(float Amount, float Duration);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged|Attack", meta=(ClampMin="0.05", Units="s"))
    float BaseAttackInterval = 0.5f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged|Upgrades", meta=(ClampMin="0.1"))
    float AttackSpeedMultiplier = 1.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged")
    TSubclassOf<AProjectileBase> ProjectileClass;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged", meta=(ClampMin="0.0", Units="cm"))
    float ProjectileSpawnDistance = 80.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged", meta=(Units="cm"))
    float ProjectileSpawnHeightOffset = 0.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged|Upgrades", meta=(ClampMin="0.0"))
    float ProjectileDamageMultiplier = 1.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged|Upgrades", meta=(ClampMin="0.0", Units="s"))
    float ProjectileLifetimeBonus = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged|Spread", meta=(ClampMin="1", ClampMax="64"))
    int32 ProjectileCount = 1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged|Spread", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg"))
    float SpreadAngle = 30.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged|Upgrades", meta=(ClampMin="0"))
    int32 PierceCount = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged|Burn")
    bool bBurnEnabled = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged|Burn", meta=(ClampMin="0.0"))
    float BurnDamage = 2.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged|Burn", meta=(ClampMin="0.1"))
    float BurnDuration = 3.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged|Burn", meta=(ClampMin="0.1"))
    float BurnPeriod = 1.f;

    float FrostStrength = 0.f;
    float FrostDuration = 2.f;
    double NextAttackTime = 0.0;
};

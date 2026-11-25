#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Projectile/ProjectileLaunchData.h"
#include "StatusEffectComponent.generated.h"
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LW_API UStatusEffectComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UStatusEffectComponent();
    void ApplyEffect(const FProjectileHitEffectSpec& Effect, AController* InstigatorController);
    void ClearEffects();
    void ApplyKnockback(const FVector& Direction, float Distance);
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Status")
    bool bBurning = false;
    bool bSlowed = false;
    float OriginalWalkSpeed = 0.f;
    float SlowStrength = 0.f;
    double SlowEndTime = 0.0;
    FVector KnockbackDirection = FVector::ZeroVector;
    float KnockbackDistance = 0.f;
    float KnockbackElapsed = 0.f;
    void ClearSlow();
    FProjectileHitEffectSpec ActiveBurn;
    TWeakObjectPtr<AController> BurnInstigator;
    double BurnEndTime = 0.0;
    double NextBurnTime = 0.0;
};

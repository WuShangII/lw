#include "StatusEffectComponent.h"
#include "Others/BurnDamageType.h"
#include "Enemy/EnemyBase.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

UStatusEffectComponent::UStatusEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UStatusEffectComponent::ApplyEffect(const FProjectileHitEffectSpec& Effect, AController* InstigatorController)
{
    AEnemyBase* Enemy = Cast<AEnemyBase>(GetOwner());
    if (!IsValid(Enemy) || !Enemy->IsAlive() || !FMath::IsFinite(Effect.Magnitude)
        || !FMath::IsFinite(Effect.Duration) || Effect.Magnitude <= 0.f || Effect.Duration <= 0.f) return;
    const double Now = GetWorld()->GetTimeSeconds();
    if (Effect.EffectId == FName(TEXT("Frost")))
    {
        if (bSlowed && Now >= SlowEndTime) ClearSlow();
        UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement();
        if (!bSlowed) OriginalWalkSpeed = Movement->MaxWalkSpeed;
        SlowStrength = FMath::Max(SlowStrength, FMath::Clamp(Effect.Magnitude, 0.f, 0.8f));
        Movement->MaxWalkSpeed = OriginalWalkSpeed * (1.f - SlowStrength);
        SlowEndTime = Now + Effect.Duration;
        bSlowed = true;
        SetComponentTickEnabled(true);
        return;
    }
    if (Effect.EffectId != FName(TEXT("Burn")) || !FMath::IsFinite(Effect.Period) || Effect.Period < 0.1f) return;
    if (!bBurning || Now >= BurnEndTime)
    {
        ActiveBurn = Effect;
        BurnInstigator = InstigatorController;
        NextBurnTime = Now + Effect.Period;
    }
    else if (Effect.Magnitude / Effect.Period > ActiveBurn.Magnitude / ActiveBurn.Period)
    {
        ActiveBurn = Effect;
        BurnInstigator = InstigatorController;
    }
    BurnEndTime = Now + Effect.Duration;
    bBurning = true;
    SetComponentTickEnabled(true);
}

void UStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    const double Now = GetWorld()->GetTimeSeconds();
    while (bBurning && NextBurnTime <= Now && NextBurnTime <= BurnEndTime + 0.0001)
    {
        NextBurnTime += ActiveBurn.Period;
        UGameplayStatics::ApplyDamage(GetOwner(), ActiveBurn.Magnitude, BurnInstigator.Get(), nullptr, UBurnDamageType::StaticClass());
        if (!IsValid(GetOwner()) || GetOwner()->IsActorBeingDestroyed()) return;
    }
    if (bBurning && Now >= BurnEndTime)
    {
        bBurning = false;
        BurnInstigator.Reset();
        ActiveBurn = FProjectileHitEffectSpec();
    }
    if (bSlowed && Now >= SlowEndTime) ClearSlow();
    if (KnockbackDistance > 0.f)
    {
        const float PreviousProgress = FMath::Clamp(KnockbackElapsed / 0.2f, 0.f, 1.f);
        KnockbackElapsed += DeltaTime;
        const float Progress = FMath::Clamp(KnockbackElapsed / 0.2f, 0.f, 1.f);
        const float Step = FMath::Square(1.f - PreviousProgress) - FMath::Square(1.f - Progress);
        FHitResult Hit;
        GetOwner()->AddActorWorldOffset(KnockbackDirection * KnockbackDistance * Step, true, &Hit);
        if (Hit.bBlockingHit || Progress >= 1.f) KnockbackDistance = 0.f;
    }
    SetComponentTickEnabled(bBurning || bSlowed || KnockbackDistance > 0.f);
}

void UStatusEffectComponent::ClearEffects()
{
    ClearSlow();
    KnockbackDistance = 0.f;
    bBurning = false;
    ActiveBurn = FProjectileHitEffectSpec();
    BurnInstigator.Reset();
    SetComponentTickEnabled(false);
}

void UStatusEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearEffects();
    Super::EndPlay(EndPlayReason);
}


void UStatusEffectComponent::ClearSlow()
{
    if (bSlowed)
    {
        if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetOwner()))
        {
            Enemy->GetCharacterMovement()->MaxWalkSpeed = OriginalWalkSpeed;
        }
    }
    bSlowed = false;
    SlowStrength = 0.f;
}

void UStatusEffectComponent::ApplyKnockback(const FVector& Direction, float Distance)
{
    const AEnemyBase* Enemy = Cast<AEnemyBase>(GetOwner());
    if (!Enemy || !Enemy->IsAlive() || !FMath::IsFinite(Distance) || Distance <= 0.f) return;
    KnockbackDirection = Direction.GetSafeNormal2D();
    KnockbackDistance = Distance;
    KnockbackElapsed = 0.f;
    SetComponentTickEnabled(true);
}

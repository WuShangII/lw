#include "RangedAttackComponent.h"
#include "PlayerStatsComponent.h"
#include "Projectile/ProjectileBase.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

URangedAttackComponent::URangedAttackComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool URangedAttackComponent::Fire(const FVector& Direction)
{
    APawn* Pawn = Cast<APawn>(GetOwner());
    const UPlayerStatsComponent* Stats = Pawn ? Pawn->FindComponentByClass<UPlayerStatsComponent>() : nullptr;
    if (!Pawn || !Stats || !ProjectileClass)
    {
        return false;
    }
    FVector FlightDirection = Direction.GetSafeNormal2D();
    if (FlightDirection.IsNearlyZero())
    {
        return false;
    }
    const FVector SpawnLocation = Pawn->GetActorLocation() + FlightDirection * ProjectileSpawnDistance
        + FVector(0.f, 0.f, ProjectileSpawnHeightOffset);
    FActorSpawnParameters Parameters;
    Parameters.Owner = Pawn;
    Parameters.Instigator = Pawn;
    Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const int32 Count = FMath::Clamp(ProjectileCount, 1, 64);
    const float TotalAngle = FMath::Clamp(SpreadAngle, 0.f, 180.f);
    bool bSpawnedAny = false;
    for (int32 Index = 0; Index < Count; ++Index)
    {
        const float Angle = Count == 1 ? 0.f : -TotalAngle * 0.5f + TotalAngle * Index / (Count - 1);
        const FVector ShotDirection = FlightDirection.RotateAngleAxis(Angle, FVector::UpVector);
        AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(
            ProjectileClass, SpawnLocation, ShotDirection.Rotation(), Parameters);
        if (!Projectile)
        {
            continue;
        }

        FProjectileLaunchData LaunchData;
        LaunchData.Damage = FMath::Max(Stats->GetAttackPower() * ProjectileDamageMultiplier * Projectile->GetDamageMultiplier(), 0.f);
        LaunchData.Lifetime = FMath::Max(Projectile->GetBaseLifetime() + ProjectileLifetimeBonus, 0.1f);
        LaunchData.Speed = FMath::Max(Projectile->GetBaseFlightSpeed(), 1.f);
        LaunchData.Direction = ShotDirection;
        LaunchData.DamageInstigator = Pawn->GetController();
        LaunchData.PierceCount = FMath::Max(PierceCount, 0);
        if (bBurnEnabled)
        {
            FProjectileHitEffectSpec Burn;
            Burn.EffectId = TEXT("Burn");
            Burn.Magnitude = BurnDamage;
            Burn.Duration = BurnDuration;
            Burn.Period = BurnPeriod;
            LaunchData.HitEffects.Add(Burn);
        }
        if (FrostStrength > 0.f)
        {
            FProjectileHitEffectSpec Frost;
            Frost.EffectId = TEXT("Frost");
            Frost.Magnitude = FrostStrength;
            Frost.Duration = FrostDuration;
            LaunchData.HitEffects.Add(Frost);
        }
        Projectile->Launch(LaunchData);
        bSpawnedAny = true;
    }
    return bSpawnedAny;
}

float URangedAttackComponent::GetAttackInterval() const
{
    return FMath::Max(BaseAttackInterval / FMath::Max(AttackSpeedMultiplier, 0.1f), 0.05f);
}

float URangedAttackComponent::GetRemainingAttackCooldown() const
{
    return static_cast<float>(FMath::Max(NextAttackTime - GetWorld()->GetTimeSeconds(), 0.0));
}

void URangedAttackComponent::CommitAttackCooldown()
{
    NextAttackTime = GetWorld()->GetTimeSeconds() + GetAttackInterval();
}

void URangedAttackComponent::AddAttackSpeedMultiplier(float Amount)
{
    if (FMath::IsFinite(Amount) && Amount > 0.f)
    {
        AttackSpeedMultiplier += Amount;
    }
}

void URangedAttackComponent::AddProjectileCount(int32 Amount)
{
    ProjectileCount = static_cast<int32>(FMath::Clamp(static_cast<int64>(ProjectileCount) + Amount, int64(1), int64(64)));
}

void URangedAttackComponent::AddPierceCount(int32 Amount)
{
    PierceCount = static_cast<int32>(FMath::Clamp(static_cast<int64>(PierceCount) + Amount, int64(0), int64(MAX_int32)));
}

void URangedAttackComponent::AddSpreadAngle(float Degrees)
{
    if (FMath::IsFinite(Degrees))
    {
        SpreadAngle = FMath::Clamp(SpreadAngle + Degrees, 0.f, 180.f);
    }
}

void URangedAttackComponent::AddProjectileDamageMultiplier(float Amount)
{
	if (FMath::IsFinite(Amount) && Amount > 0.f)
	{
		ProjectileDamageMultiplier += Amount;
	}
}

void URangedAttackComponent::AddProjectileLifetimeBonus(float Seconds)
{
	if (FMath::IsFinite(Seconds) && Seconds > 0.f)
	{
		ProjectileLifetimeBonus += Seconds;
	}
}


void URangedAttackComponent::SetBurnEffect(float DamagePerTick, float Duration, float Period)
{
    if (!FMath::IsFinite(DamagePerTick) || !FMath::IsFinite(Duration) || !FMath::IsFinite(Period)) return;
    BurnDamage = FMath::Max(DamagePerTick, 0.f);
    BurnDuration = FMath::Max(Duration, 0.1f);
    BurnPeriod = FMath::Max(Period, 0.1f);
    bBurnEnabled = BurnDamage > 0.f;
}

void URangedAttackComponent::AddBurnDamage(float Amount, float Duration, float Period)
{
    SetBurnEffect((bBurnEnabled ? BurnDamage : 0.f) + Amount, Duration, Period);
}

void URangedAttackComponent::AddFrostStrength(float Amount, float Duration)
{
    FrostStrength = FMath::Clamp(FrostStrength + Amount, 0.f, 0.8f);
    FrostDuration = Duration;
}

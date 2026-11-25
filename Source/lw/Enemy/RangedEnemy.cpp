#include "RangedEnemy.h"
#include "Projectile/ProjectileBase.h"
#include "AIController.h"
#include "Engine/World.h"

void ARangedEnemy::ExecuteAttackHit(AActor* Target)
{
	if (!IsValid(Target) || !ProjectileClass) return;
	AAIController* AI = Cast<AAIController>(GetController());
	if (AI && !AI->LineOfSightTo(Target)) return;
	const FVector AimDirection = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (AimDirection.IsNearlyZero()) return;
	SetActorRotation(AimDirection.Rotation());
	const FVector SpawnLocation = GetActorLocation() + AimDirection * ProjectileSpawnDistance
		+ FVector(0.f, 0.f, ProjectileSpawnHeightOffset);
	const FVector FlightDirection = (Target->GetActorLocation() - SpawnLocation).GetSafeNormal();
	FActorSpawnParameters Parameters;
	Parameters.Owner = this;
	Parameters.Instigator = this;
	Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(
		ProjectileClass, SpawnLocation, FlightDirection.Rotation(), Parameters);
	if (!Projectile) return;
	FProjectileLaunchData Data;
	Data.bTargetsPlayer = true;
	Data.Damage = FMath::Max(AttackDamage * Projectile->GetDamageMultiplier(), 0.f);
	Data.Speed = FMath::Max(Projectile->GetBaseFlightSpeed(), 1.f);
	Data.Lifetime = FMath::Max(Projectile->GetBaseLifetime(), 0.1f);
	Data.Direction = FlightDirection;
	Data.DamageInstigator = GetController();
	Projectile->Launch(Data);
}

#include "MeleeEnemy.h"
#include "Player/lwCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "AIController.h"

void AMeleeEnemy::ExecuteAttackHit(AActor* Target)
{
	AlwCharacter* Player = Cast<AlwCharacter>(Target);
	if (!Player || !IsTargetInAttackRange(Player)) return;
	const FVector Direction = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	const float MinimumDot = FMath::Cos(FMath::DegreesToRadians(AttackHalfAngle));
	if (!Direction.IsNearlyZero() && FVector::DotProduct(GetActorForwardVector(), Direction) < MinimumDot) return;
	AAIController* AI = Cast<AAIController>(GetController());
	if (AI && !AI->LineOfSightTo(Player)) return;
	UGameplayStatics::ApplyDamage(Player, AttackDamage, GetController(), this, UDamageType::StaticClass());
}

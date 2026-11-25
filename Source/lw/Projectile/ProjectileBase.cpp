// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "Enemy/EnemyBase.h"
#include "Player/lwCharacter.h"
#include "Components/StatusEffectComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

namespace
{
	constexpr ECollisionChannel ProjectileChannel = ECC_GameTraceChannel1;
}

// Sets default values
AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);
	SphereCollision->InitSphereRadius(12.f);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ProjectileChannel);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SphereCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollision->SetGenerateOverlapEvents(true);
	SphereCollision->SetCanEverAffectNavigation(false);

	VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
	VisualRoot->SetupAttachment(SphereCollision);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(VisualRoot);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetSimulatePhysics(false);
	ProjectileMesh->SetGenerateOverlapEvents(false);
	ProjectileMesh->SetCanEverAffectNavigation(false);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(SphereCollision);
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	SphereCollision->SetCollisionObjectType(ProjectileChannel);
	SphereCollision->SetCollisionResponseToChannel(ProjectileChannel, ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollision->SetGenerateOverlapEvents(true);
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnProjectileOverlap);
	SetLifeSpan(FMath::Max(Lifetime, 0.1f));
	SphereCollision->IgnoreActorWhenMoving(GetOwner(), true);
	if (AActor* OwnerActor = GetOwner())
	{
		TArray<AActor*> AttachedActors;
		OwnerActor->GetAttachedActors(AttachedActors);
		for (AActor* AttachedActor : AttachedActors)
		{
			SphereCollision->IgnoreActorWhenMoving(AttachedActor, true);
		}
	}
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &AProjectileBase::OnProjectileStopped);
}

void AProjectileBase::Launch(const FProjectileLaunchData& InLaunchData)
{
	LaunchData = InLaunchData;
	RemainingPierces = FMath::Max(LaunchData.PierceCount, 0);
	bLaunched = true;
	LaunchData.Direction = LaunchData.Direction.GetSafeNormal();
	SetLifeSpan(LaunchData.Lifetime);
	ProjectileMovement->MaxSpeed = LaunchData.Speed;
	ProjectileMovement->Velocity = LaunchData.Direction * LaunchData.Speed;

	// 生成时已重叠的目标，在伤害数据就绪后处理。
	TArray<UPrimitiveComponent*> Overlaps;
	SphereCollision->GetOverlappingComponents(Overlaps);
	for (UPrimitiveComponent* Component : Overlaps)
	{
		if (bHitProcessed)
		{
			break;
		}
		if (IsValid(Component))
		{
			FHitResult Hit(Component->GetOwner(), Component, GetActorLocation(), -LaunchData.Direction);
			ProcessTargetHit(Component->GetOwner(), Hit);
		}
	}
}

void AProjectileBase::OnProjectileStopped(const FHitResult& Hit)
{
	if (bHitProcessed)
	{
		return;
	}
	bHitProcessed = true;
	Destroy();
}

void AProjectileBase::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const FHitResult Hit = bFromSweep ? SweepResult
		: FHitResult(OtherActor, OtherComp, GetActorLocation(), -LaunchData.Direction);
	ProcessTargetHit(OtherActor, Hit);
}

void AProjectileBase::ProcessTargetHit(AActor* OtherActor, const FHitResult& Hit)
{
	AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
	const bool bValidTarget = LaunchData.bTargetsPlayer
		? IsValid(Cast<AlwCharacter>(OtherActor)) : IsValid(Enemy);
	if (!bLaunched || bHitProcessed || !bValidTarget || OtherActor == GetOwner()
		|| HitActors.Contains(TWeakObjectPtr<AActor>(OtherActor)))
	{
		return;
	}
	HitActors.Add(OtherActor);
	SphereCollision->IgnoreActorWhenMoving(OtherActor, true);
	const bool bDestroyAfterHit = RemainingPierces == 0;
	if (bDestroyAfterHit)
	{
		bHitProcessed = true;
	}
	else
	{
		--RemainingPierces;
	}
	UGameplayStatics::ApplyPointDamage(OtherActor, LaunchData.Damage, LaunchData.Direction, Hit,
		LaunchData.DamageInstigator.Get(), this, UDamageType::StaticClass());
	if (IsValid(Enemy) && Enemy->IsAlive())
	{
		if (UStatusEffectComponent* Status = Enemy->FindComponentByClass<UStatusEffectComponent>())
		{
			for (const FProjectileHitEffectSpec& Effect : LaunchData.HitEffects)
			{
				Status->ApplyEffect(Effect, LaunchData.DamageInstigator.Get());
			}
		}
	}
	if (bDestroyAfterHit)
	{
		Destroy();
	}
}


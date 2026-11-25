// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "Others/BurnDamageType.h"
#include "Player/lwPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StatusEffectComponent.h"
#include "Components/PlayerStatsComponent.h"
#include "GameFramework/Controller.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "TimerManager.h"

float AEnemyBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead || CurrentHealth <= 0.f || !FMath::IsFinite(DamageAmount) || DamageAmount <= 0.f || !CanBeDamaged())
	{
		return 0.f;
	}
	const float AppliedDamage = FMath::Min(DamageAmount, CurrentHealth);
	CurrentHealth -= AppliedDamage;
    if (AlwPlayerController* PC = Cast<AlwPlayerController>(EventInstigator))
    {
        const bool bBurn = DamageEvent.DamageTypeClass && DamageEvent.DamageTypeClass->IsChildOf(UBurnDamageType::StaticClass());
        const FVector NumberLocation = GetActorLocation() + FVector::UpVector * (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 20.f);
        PC->ShowDamageNumber(AppliedDamage, NumberLocation, bBurn);
    }
	Super::TakeDamage(AppliedDamage, DamageEvent, EventInstigator, DamageCauser);
	if (CurrentHealth <= 0.f)
	{
		Die(EventInstigator);
	}
	else if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		BeginHitStun();
	}
	return AppliedDamage;
}

void AEnemyBase::Die(AController* KillerController)
{
	if (bIsDead)
	{
		return;
	}
	bIsDead = true;
	GetWorldTimerManager().ClearTimer(HitStunTimerHandle);
	CancelAttack();
	StatusEffectComponent->ClearEffects();

	APawn* Killer = IsValid(KillerController) ? KillerController->GetPawn() : nullptr;
	if (IsValid(Killer))
	{
		if (UPlayerStatsComponent* Stats = Killer->FindComponentByClass<UPlayerStatsComponent>())
		{
			Stats->RestoreHealthOnKill();
			Stats->AddExperience(ExperienceReward);
		}
	}
	Destroy();
}

// Sets default values
AEnemyBase::AEnemyBase()
{
	StatusEffectComponent = CreateDefaultSubobject<UStatusEffectComponent>(TEXT("StatusEffectComponent"));
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	CurrentHealth = MaxHealth;
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}



bool AEnemyBase::IsTargetInAttackRange(const AActor* Target) const
{
	return IsValid(Target) && FVector::DistSquared(GetActorLocation(), Target->GetActorLocation()) <= FMath::Square(AttackRange);
}

bool AEnemyBase::CanAttack() const
{
	return IsAlive() && !bIsStunned && !bIsAttacking && AttackMontage
		&& GetWorld()->GetTimeSeconds() >= NextAttackTime;
}

bool AEnemyBase::StartAttack(AActor* Target)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!CanAttack() || !IsTargetInAttackRange(Target) || !AnimInstance)
	{
		return false;
	}
	if (AnimInstance->Montage_Play(AttackMontage) <= 0.f)
	{
		return false;
	}
	bIsAttacking = true;
	bAttackHitPending = true;
	AttackTarget = Target;
	NextAttackTime = GetWorld()->GetTimeSeconds() + AttackCooldown;
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
	}
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->StopMovementImmediately();
	bOrientBeforeAttack = Movement->bOrientRotationToMovement;
	bDesiredRotationBeforeAttack = Movement->bUseControllerDesiredRotation;
	Movement->bOrientRotationToMovement = false;
	Movement->bUseControllerDesiredRotation = false;
	const FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (!Direction.IsNearlyZero()) SetActorRotation(Direction.Rotation());
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AEnemyBase::OnAttackEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
	FOnMontageBlendingOutStarted BlendDelegate;
	BlendDelegate.BindUObject(this, &AEnemyBase::OnAttackBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendDelegate, AttackMontage);
	return true;
}

void AEnemyBase::CancelAttack()
{
	if (!bIsAttacking) return;
	bAttackHitPending = false;
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(0.f, AttackMontage);
	}
	OnAttackEnded(AttackMontage, true);
}

void AEnemyBase::OnAttackEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bIsAttacking) return;
	bIsAttacking = false;
	bAttackHitPending = false;
	AttackTarget.Reset();
	GetCharacterMovement()->bOrientRotationToMovement = bOrientBeforeAttack;
	GetCharacterMovement()->bUseControllerDesiredRotation = bDesiredRotationBeforeAttack;
}

void AEnemyBase::OnAttackBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted) bAttackHitPending = false;
}

void AEnemyBase::NotifyAttackHit()
{
	if (!bIsAttacking || !bAttackHitPending || bIsStunned || !IsAlive()) return;
	bAttackHitPending = false;
	if (AttackTarget.IsValid()) ExecuteAttackHit(AttackTarget.Get());
}

void AEnemyBase::ExecuteAttackHit(AActor* Target)
{
	// 下一阶段由近战、远程子类实现命中效果。
}


void AEnemyBase::BeginHitStun()
{
	if (!IsAlive() || HitStunDuration <= 0.f) return;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!bIsStunned)
	{
		MovementModeBeforeStun = Movement->MovementMode;
		CustomMovementModeBeforeStun = Movement->CustomMovementMode;
	}
	bIsStunned = true;
	CancelAttack();
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
		if (UBlackboardComponent* Blackboard = AI->GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool(TEXT("IsStunned"), true);
		}
	}
	Movement->StopMovementImmediately();
	Movement->DisableMovement();
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (HitReactMontage) AnimInstance->Montage_Play(HitReactMontage);
	}
	GetWorldTimerManager().SetTimer(HitStunTimerHandle, this, &AEnemyBase::EndHitStun, HitStunDuration, false);
}

void AEnemyBase::EndHitStun()
{
	if (!IsAlive()) return;
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (HitReactMontage) AnimInstance->Montage_Stop(0.1f, HitReactMontage);
	}
	GetCharacterMovement()->SetMovementMode(MovementModeBeforeStun, CustomMovementModeBeforeStun);
	bIsStunned = false;
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* Blackboard = AI->GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool(TEXT("IsStunned"), false);
		}
	}
}

void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(HitStunTimerHandle);
	Super::EndPlay(EndPlayReason);
}

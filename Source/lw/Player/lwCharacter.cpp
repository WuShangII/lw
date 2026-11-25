// Copyright Epic Games, Inc. All Rights Reserved.

#include "lwCharacter.h"
#include "lwPlayerController.h"
#include "Others/WeaponBase.h"
#include "Components/RangedAttackComponent.h"
#include "Components/PlayerUpgradeComponent.h"
#include "Enemy/EnemyBase.h"
#include "Components/StatusEffectComponent.h"
#include "Components/PlayerStatsComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "InputAction.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "lw.h"
#include "Others/lwGameMode.h"

AlwCharacter::AlwCharacter()
{
	PlayerUpgradeComponent = CreateDefaultSubobject<UPlayerUpgradeComponent>(TEXT("PlayerUpgradeComponent"));
	RangedAttackComponent = CreateDefaultSubobject<URangedAttackComponent>(TEXT("RangedAttackComponent"));
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Character facing is independent of the fixed camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanJump = false;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a fixed-angle camera boom that follows the character's position.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	ConfigureTopDownCamera();

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AlwCharacter::ConfigureTopDownCamera()
{
	// Absolute rotation prevents the camera from turning with the character.
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->SetWorldRotation(TopDownCameraRotation);
	CameraBoom->TargetArmLength = TopDownCameraDistance;
	// Arena obstacles must not push the top-down camera toward the player.
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = false;
	CameraBoom->bEnableCameraRotationLag = false;
	FollowCamera->bUsePawnControlRotation = false;
}

void AlwCharacter::BeginPlay()
{
	Super::BeginPlay();
	BaseWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	// Blueprint assets may still contain the third-person component defaults.
	ConfigureTopDownCamera();
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanJump = false;
	StopJumping();
	SpawnWeapon();
}

void AlwCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(EquippedWeapon))
	{
		EquippedWeapon->Destroy();
		EquippedWeapon = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AlwCharacter::SpawnWeapon()
{
	if (!WeaponClass || WeaponSocketName.IsNone() || !GetMesh()->DoesSocketExist(WeaponSocketName))
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	EquippedWeapon = GetWorld()->SpawnActor<AWeaponBase>(
		WeaponClass,
		GetMesh()->GetSocketLocation(WeaponSocketName),
		GetMesh()->GetSocketRotation(WeaponSocketName),
		SpawnParameters);

	if (EquippedWeapon && !EquippedWeapon->AttachToComponent(
		GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName))
	{
		EquippedWeapon->Destroy();
		EquippedWeapon = nullptr;
	}
}

bool AlwCharacter::CanJumpInternal_Implementation() const
{
	return false;
}

void AlwCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AlwCharacter::Move);
		if (RangedAttackAction)
		{
			EnhancedInputComponent->BindAction(RangedAttackAction, ETriggerEvent::Started, this, &AlwCharacter::OnRangedAttack);
		}
		if (MeleeAttackAction)
		{
			EnhancedInputComponent->BindAction(MeleeAttackAction, ETriggerEvent::Started, this, &AlwCharacter::OnMeleeAttack);
		}
	}
	else
	{
		UE_LOG(Loglw, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AlwCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AlwCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// Move relative to the camera, regardless of controller/spawn rotation.
		const FRotator Rotation = CameraBoom->GetComponentRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

bool AlwCharacter::GetAttackAimLocation(FVector& TargetLocation) const
{
	const AlwPlayerController* PlayerController = Cast<AlwPlayerController>(GetController());
	const float GroundHeight = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	return PlayerController && PlayerController->GetMouseAimLocation(GroundHeight, TargetLocation);
}

void AlwCharacter::FaceAttackTarget(const FVector& TargetLocation)
{
	bOrientRotationBeforeAttack = GetCharacterMovement()->bOrientRotationToMovement;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	const FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal2D();
	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}
}

void AlwCharacter::FinishAttack()
{
	bIsAttacking = false;
	GetCharacterMovement()->bOrientRotationToMovement = bOrientRotationBeforeAttack;
}

void AlwCharacter::OnRangedAttack()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (bIsAttacking || !AnimInstance || !RangedAttackMontage
		|| RangedAttackComponent->GetRemainingAttackCooldown() > 0.f)
	{
		return;
	}
	FVector TargetLocation;
	if (!GetAttackAimLocation(TargetLocation))
	{
		return;
	}
	const float PlayRate = FMath::Max(RangedMontageBasePlayRate, 0.01f)
		* RangedAttackComponent->GetAttackSpeedMultiplier();
	if (AnimInstance->Montage_Play(RangedAttackMontage, PlayRate) <= 0.f)
	{
		return;
	}
	bIsAttacking = true;
	bRangedAttackActive = true;
	bRangedProjectilePending = true;
	RangedAttackComponent->CommitAttackCooldown();
	FaceAttackTarget(TargetLocation);
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AlwCharacter::OnRangedAttackEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, RangedAttackMontage);
	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &AlwCharacter::OnRangedAttackBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, RangedAttackMontage);
}

void AlwCharacter::FireRangedProjectile()
{
	if (!bRangedAttackActive || !bRangedProjectilePending)
	{
		return;
	}
	bRangedProjectilePending = false;
	RangedAttackComponent->Fire(GetActorForwardVector());
}

void AlwCharacter::OnRangedAttackBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		bRangedProjectilePending = false;
	}
}

void AlwCharacter::OnRangedAttackEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bRangedProjectilePending = false;
	bRangedAttackActive = false;
	FinishAttack();
}

void AlwCharacter::OnMeleeAttack()
{
	if (bIsAttacking)
	{
		return;
	}

	FVector TargetLocation;
	if (GetAttackAimLocation(TargetLocation))
	{
		StartMeleeAttack(TargetLocation);
	}
}

void AlwCharacter::StartMeleeAttack(const FVector& TargetLocation)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (bIsAttacking || !AnimInstance || !MeleeAttackMontage)
	{
		return;
	}

	if (AnimInstance->Montage_Play(MeleeAttackMontage, MeleeSpeedMultiplier) <= 0.0f)
	{
		return;
	}

	bIsAttacking = true;
	HitActors.Reset();
	FaceAttackTarget(TargetLocation);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AlwCharacter::OnMeleeAttackEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, MeleeAttackMontage);
	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &AlwCharacter::OnMeleeAttackBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, MeleeAttackMontage);
}

void AlwCharacter::OnMeleeAttackBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		EndMeleeTrace();
	}
}

void AlwCharacter::OnMeleeAttackEnded(UAnimMontage* Montage, bool bInterrupted)
{
	EndMeleeTrace();
	HitActors.Reset();
	FinishAttack();
}

void AlwCharacter::BeginMeleeTrace()
{
	if (bIsAttacking && !bRangedAttackActive && IsValid(EquippedWeapon))
	{
		EquippedWeapon->BeginTrace();
		UpdateMeleeTrace();
	}
}

void AlwCharacter::UpdateMeleeTrace()
{
	if (!bIsAttacking || bRangedAttackActive || !IsValid(EquippedWeapon))
	{
		return;
	}
	TArray<FHitResult> Hits;
	EquippedWeapon->UpdateTrace(Hits);
	const UPlayerStatsComponent* Stats = FindComponentByClass<UPlayerStatsComponent>();
	if (!Stats)
	{
		return;
	}
	for (const FHitResult& Hit : Hits)
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(Hit.GetActor());
		if (!IsValid(Enemy) || HitActors.Contains(TWeakObjectPtr<AActor>(Enemy)))
		{
			continue;
		}
		HitActors.Add(Enemy);
		UGameplayStatics::ApplyPointDamage(Enemy, Stats->GetAttackPower() * MeleeDamageMultiplier,
			(Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal(), Hit,
            GetController(), EquippedWeapon, UDamageType::StaticClass());
        if (IsValid(Enemy) && Enemy->IsAlive() && MeleeKnockbackDistance > 0.f)
        {
            if (UStatusEffectComponent* Status = Enemy->FindComponentByClass<UStatusEffectComponent>())
            {
                Status->ApplyKnockback(Enemy->GetActorLocation() - GetActorLocation(), MeleeKnockbackDistance);
            }
        }
	}
}

void AlwCharacter::EndMeleeTrace()
{
	if (IsValid(EquippedWeapon))
	{
		EquippedWeapon->EndTrace();
	}
}

void AlwCharacter::DoLook(float Yaw, float Pitch)
{
	// Fixed-angle camera: template touch/UI look input is intentionally ignored.
}

void AlwCharacter::DoJumpStart()
{
	// This demo has no jump action, including through template touch/UI input.
}

void AlwCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

float AlwCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	UPlayerStatsComponent* Stats = FindComponentByClass<UPlayerStatsComponent>();
	if (!HasAuthority() || !CanBeDamaged() || !Stats || !Stats->IsAlive()) return 0.f;
	const float AppliedDamage = Stats->ApplyHealthDamage(DamageAmount);
	if (AppliedDamage <= 0.f) return 0.f;
	if (!Stats->IsAlive())
	{
		SetCanBeDamaged(false);
		if (AlwGameMode* GameMode = GetWorld()->GetAuthGameMode<AlwGameMode>())
		{
			GameMode->HandlePlayerDefeated();
		}
	}
	Super::TakeDamage(AppliedDamage, DamageEvent, EventInstigator, DamageCauser);
	return AppliedDamage;
}

void AlwCharacter::AddMeleeDamageMultiplier(float Amount)
{
	MeleeDamageMultiplier += Amount;
}

void AlwCharacter::AddMeleeSpeedMultiplier(float Amount)
{
	MeleeSpeedMultiplier += Amount;
}

void AlwCharacter::AddMoveSpeedMultiplier(float Amount)
{
	MoveSpeedBonus += Amount;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * (1.f + MoveSpeedBonus);
}

void AlwCharacter::AddMeleeKnockback(float Amount)
{
    MeleeKnockbackDistance += Amount;
}

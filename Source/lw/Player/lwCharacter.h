// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "lwCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UAnimMontage;
class AWeaponBase;
class URangedAttackComponent;
class UPlayerUpgradeComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  Player character with a fixed-angle top-down follow camera and no jumping.
 */
UCLASS(abstract)
class AlwCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Fixed camera orientation in world space. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|Top Down")
	FRotator TopDownCameraRotation = FRotator(-60.0f, 0.0f, 0.0f);

	/** Distance from the character to the camera, in centimeters. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|Top Down", meta=(ClampMin="100.0", Units="cm"))
	float TopDownCameraDistance = 1200.0f;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> MeleeAttackAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> RangedAttackAction;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<URangedAttackComponent> RangedAttackComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPlayerUpgradeComponent> PlayerUpgradeComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	TObjectPtr<UAnimMontage> MeleeAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Ranged")
	TObjectPtr<UAnimMontage> RangedAttackMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Ranged", meta=(ClampMin="0.01"))
	float RangedMontageBasePlayRate = 1.f;
	bool bRangedAttackActive = false;
	bool bRangedProjectilePending = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat")
	bool bIsAttacking = false;

	bool bOrientRotationBeforeAttack = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Weapon")
	TSubclassOf<AWeaponBase> WeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Weapon")
	FName WeaponSocketName = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Weapon")
	TObjectPtr<AWeaponBase> EquippedWeapon;

public:

	/** Constructor */
	AlwCharacter();
	void AddMeleeDamageMultiplier(float Amount);
	void AddMeleeSpeedMultiplier(float Amount);
	void AddMoveSpeedMultiplier(float Amount);
	void AddMeleeKnockback(float Amount);
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;	
	void BeginMeleeTrace();
	void UpdateMeleeTrace();
	void EndMeleeTrace();
	void FireRangedProjectile();

protected:

	virtual void BeginPlay() override;
	float MeleeKnockbackDistance = 0.f;
	float MeleeDamageMultiplier = 1.f;
	float MeleeSpeedMultiplier = 1.f;
	float BaseWalkSpeed = 0.f;
	float MoveSpeedBonus = 0.f;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool CanJumpInternal_Implementation() const override;

	/** Apply after blueprint defaults have loaded as well as in the constructor. */
	void ConfigureTopDownCamera();

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	void OnMeleeAttack();
	void OnRangedAttack();
	void OnRangedAttackEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnRangedAttackBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	bool GetAttackAimLocation(FVector& TargetLocation) const;
	void FaceAttackTarget(const FVector& TargetLocation);
	void FinishAttack();
	void StartMeleeAttack(const FVector& TargetLocation);
	void OnMeleeAttackEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnMeleeAttackBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void SpawnWeapon();
	TSet<TWeakObjectPtr<AActor>> HitActors;

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Retained for template UI compatibility; fixed camera ignores look input. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Retained for template UI compatibility; jumping is disabled. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};


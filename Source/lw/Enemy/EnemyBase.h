// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class UStatusEffectComponent;
class UAnimMontage;

UCLASS()
class LW_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();
	bool StartAttack(AActor* Target);
	void CancelAttack();
	void NotifyAttackHit();
	bool IsAttacking() const { return bIsAttacking; }
	bool IsStunned() const { return bIsStunned; }
	bool IsTargetInAttackRange(const AActor* Target) const;
	bool CanAttack() const;
	bool IsAlive() const { return !bIsDead && CurrentHealth > 0.f; }
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|HitReact", meta=(ClampMin="0.0", Units="s"))
	float HitStunDuration = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|HitReact")
	TObjectPtr<UAnimMontage> HitReactMontage;
	FTimerHandle HitStunTimerHandle;
	EMovementMode MovementModeBeforeStun = MOVE_Walking;
	uint8 CustomMovementModeBeforeStun = 0;
	void BeginHitStun();
	void EndHitStun();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Attack", meta=(ClampMin="0.0"))
	float AttackDamage = 10.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Attack", meta=(ClampMin="1.0", Units="cm"))
	float AttackRange = 180.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Attack", meta=(ClampMin="0.1", Units="s"))
	float AttackCooldown = 1.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Attack")
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Enemy|Attack")
	bool bIsAttacking = false;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Enemy|Attack")
	bool bIsStunned = false;
	TWeakObjectPtr<AActor> AttackTarget;
	bool bAttackHitPending = false;
	bool bOrientBeforeAttack = false;
	bool bDesiredRotationBeforeAttack = false;
	double NextAttackTime = 0.0;
	virtual void ExecuteAttackHit(AActor* Target);
	void OnAttackEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnAttackBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 30.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|Health")
	float CurrentHealth = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Rewards", meta = (ClampMin = "0.0"))
	float ExperienceReward = 5.f;

	bool bIsDead = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStatusEffectComponent> StatusEffectComponent;
	void Die(AController* KillerController);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};

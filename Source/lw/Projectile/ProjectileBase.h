// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileLaunchData.h"
#include "ProjectileBase.generated.h"

class USphereComponent;
class USceneComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class LW_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileBase();
	void Launch(const FProjectileLaunchData& InLaunchData);
	float GetBaseFlightSpeed() const { return FlightSpeed; }
	float GetBaseLifetime() const { return Lifetime; }
	float GetDamageMultiplier() const { return DamageMultiplier; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
	TObjectPtr<USphereComponent> SphereCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
	TObjectPtr<USceneComponent> VisualRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile", meta=(ClampMin="1.0"))
	float FlightSpeed = 1200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile", meta=(ClampMin="0.1"))
	float Lifetime = 3.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile", meta=(ClampMin="0.0"))
	float DamageMultiplier = 1.f;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Projectile")
	FProjectileLaunchData LaunchData;
	bool bHitProcessed = false;
	bool bLaunched = false;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Projectile")
	int32 RemainingPierces = 0;
	TSet<TWeakObjectPtr<AActor>> HitActors;

	void ProcessTargetHit(AActor* OtherActor, const FHitResult& Hit);
	UFUNCTION()
	void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnProjectileStopped(const FHitResult& Hit);

};

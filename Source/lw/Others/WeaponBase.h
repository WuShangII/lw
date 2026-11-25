// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class UStaticMeshComponent;

UCLASS()
class LW_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:
	AWeaponBase();
	void BeginTrace();
	void EndTrace();
	void UpdateTrace(TArray<FHitResult>& OutHits);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Trace")
	FName TraceStartSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Trace")
	FName TraceEndSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Trace", meta=(ClampMin="1.0"))
	float TraceRadius = 12.f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Trace")
	bool bDrawDebugTrace = false;

	bool bTraceActive = false;
	FVector PreviousStart;
	FVector PreviousEnd;
};

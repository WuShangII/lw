// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetGenerateOverlapEvents(false);
	WeaponMesh->SetCanEverAffectNavigation(false);
}

void AWeaponBase::BeginTrace()
{
	bTraceActive = WeaponMesh->DoesSocketExist(TraceStartSocket) && WeaponMesh->DoesSocketExist(TraceEndSocket)
		&& !TraceStartSocket.IsNone() && !TraceEndSocket.IsNone();
	if (bTraceActive)
	{
		PreviousStart = WeaponMesh->GetSocketLocation(TraceStartSocket);
		PreviousEnd = WeaponMesh->GetSocketLocation(TraceEndSocket);
	}
}

void AWeaponBase::EndTrace()
{
	bTraceActive = false;
}

void AWeaponBase::UpdateTrace(TArray<FHitResult>& OutHits)
{
	OutHits.Reset();
	if (!bTraceActive)
	{
		return;
	}
	const FVector Start = WeaponMesh->GetSocketLocation(TraceStartSocket);
	const FVector End = WeaponMesh->GetSocketLocation(TraceEndSocket);
	const float Radius = FMath::Max(TraceRadius, 1.f);
	const int32 Steps = FMath::Max(1, FMath::CeilToInt(FVector::Distance(Start, End) / Radius));
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_Pawn);
	for (int32 Index = 0; Index <= Steps; ++Index)
	{
		const float Alpha = static_cast<float>(Index) / Steps;
		const FVector From = FMath::Lerp(PreviousStart, PreviousEnd, Alpha);
		const FVector To = FMath::Lerp(Start, End, Alpha);
		TArray<FHitResult> Hits;
		GetWorld()->SweepMultiByObjectType(Hits, From, To, FQuat::Identity, Objects,
			FCollisionShape::MakeSphere(Radius), Params);
		OutHits.Append(Hits);
		if (bDrawDebugTrace)
		{
			DrawDebugLine(GetWorld(), From, To, FColor::Yellow, false, 0.15f);
			DrawDebugSphere(GetWorld(), To, Radius, 8, FColor::Green, false, 0.15f);
		}
	}
	PreviousStart = Start;
	PreviousEnd = End;
}


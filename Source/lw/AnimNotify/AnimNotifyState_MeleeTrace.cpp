// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_MeleeTrace.h"
#include "Player/lwCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_MeleeTrace::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (AlwCharacter* Character = Cast<AlwCharacter>(MeshComp->GetOwner()))
	{
		Character->BeginMeleeTrace();
	}
}

void UAnimNotifyState_MeleeTrace::NotifyTick(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (AlwCharacter* Character = Cast<AlwCharacter>(MeshComp->GetOwner()))
	{
		Character->UpdateMeleeTrace();
	}
}

void UAnimNotifyState_MeleeTrace::NotifyEnd(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AlwCharacter* Character = Cast<AlwCharacter>(MeshComp->GetOwner()))
	{
		Character->EndMeleeTrace();
	}
}


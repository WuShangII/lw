#include "AnimNotify_FireProjectile.h"
#include "Player/lwCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_FireProjectile::Notify(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (AlwCharacter* Character = Cast<AlwCharacter>(MeshComp->GetOwner()))
	{
		Character->FireRangedProjectile();
	}
}

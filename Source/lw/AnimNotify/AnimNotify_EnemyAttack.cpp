#include "AnimNotify_EnemyAttack.h"
#include "Enemy/EnemyBase.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_EnemyAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(MeshComp->GetOwner()))
	{
		Enemy->NotifyAttackHit();
	}
}

#include "BTService_UpdateEnemyCombat.h"
#include "AIController.h"
#include "Enemy/EnemyBase.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_UpdateEnemyCombat::UBTService_UpdateEnemyCombat()
{
	NodeName = TEXT("Update Enemy Combat");
	bNotifyTick = true;
	bCallTickOnSearchStart = true;
	Interval = 0.1f;
	RandomDeviation = 0.f;
}

void UBTService_UpdateEnemyCombat::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	AAIController* AI = OwnerComp.GetAIOwner();
	AEnemyBase* Enemy = AI ? Cast<AEnemyBase>(AI->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Enemy || !Blackboard) return;
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("TargetActor")));
	Blackboard->SetValueAsBool(TEXT("IsStunned"), Enemy->IsStunned());
	Blackboard->SetValueAsBool(TEXT("CanAttack"), Enemy->CanAttack());
	// 攻击期间保持分支，等蒙太奇结束后再按距离切换追踪。
	Blackboard->SetValueAsBool(TEXT("InAttackRange"),
		Enemy->IsAttacking() || Enemy->IsTargetInAttackRange(Target));
}

#include "BTTask_EnemyAttack.h"
#include "AIController.h"
#include "Enemy/EnemyBase.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_EnemyAttack::UBTTask_EnemyAttack()
{
	NodeName = TEXT("Enemy Attack");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_EnemyAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	AEnemyBase* Enemy = AI ? Cast<AEnemyBase>(AI->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* Target = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TEXT("TargetActor"))) : nullptr;
	return Enemy && Enemy->StartAttack(Target) ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
}

void UBTTask_EnemyAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	AEnemyBase* Enemy = AI ? Cast<AEnemyBase>(AI->GetPawn()) : nullptr;
	if (!Enemy || !Enemy->IsAlive())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
	else if (!Enemy->IsAttacking())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_EnemyAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if (AEnemyBase* Enemy = AI ? Cast<AEnemyBase>(AI->GetPawn()) : nullptr)
	{
		Enemy->CancelAttack();
	}
	return EBTNodeResult::Aborted;
}

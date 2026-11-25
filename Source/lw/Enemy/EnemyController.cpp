// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "lw.h"

void AEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!BehaviorTreeAsset)
	{
		UE_LOG(Loglw, Warning,
			TEXT("%s: No BehaviorTreeAsset assigned. Set it in the controller blueprint's Class Defaults."),
			*GetNameSafe(this));
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	UBlackboardComponent* BlackboardComponent = nullptr;
	if (!UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BlackboardComponent))
	{
		UE_LOG(Loglw, Error, TEXT("%s: Failed to initialize blackboard for %s."), *GetNameSafe(InPawn), *GetNameSafe(BehaviorTreeAsset));
		return;
	}
	BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), PlayerPawn);

	if (!RunBehaviorTree(BehaviorTreeAsset))
	{
		UE_LOG(Loglw, Error, TEXT("%s: Failed to start behavior tree %s for pawn %s."),
			*GetNameSafe(this), *GetNameSafe(BehaviorTreeAsset), *GetNameSafe(InPawn));
	}
}


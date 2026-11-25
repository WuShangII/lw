// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "lwGameMode.generated.h"

class AEnemySpawner;

UENUM(BlueprintType)
enum class ESurvivalMatchState : uint8
{
	Spawning,
	WaitingForClear,
	Victory,
	VictorySlowMotion,
	Defeat
};

UCLASS(abstract)
class AlwGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AlwGameMode();

	void HandlePlayerDefeated();
	bool CanPauseMatch() const
	{
		return MatchState == ESurvivalMatchState::Spawning || MatchState == ESurvivalMatchState::WaitingForClear;
	}
	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category="Match")
	int32 GetTotalAliveEnemyCount() const;

	UFUNCTION(BlueprintPure, Category="Match")
	float GetElapsedMatchTime() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Match", meta=(ClampMin="0.1", Units="s"))
	float SpawnDuration = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Match|Victory", meta=(ClampMin="0.01", ClampMax="1.0"))
	float VictoryTimeDilation = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Match|Victory", meta=(ClampMin="0.0", Units="s"))
	float VictorySlowMotionDuration = 2.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Match")
	ESurvivalMatchState MatchState = ESurvivalMatchState::Spawning;

	UFUNCTION(BlueprintImplementableEvent, Category="Match")
	void OnVictory();

	UFUNCTION(BlueprintImplementableEvent, Category="Match")
	void OnVictoryFinished();

	UFUNCTION(BlueprintImplementableEvent, Category="Match")
	void OnDefeat();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	TArray<TWeakObjectPtr<AEnemySpawner>> EnemySpawners;
	FTimerHandle SpawnEndTimerHandle;
	double MatchStartTime = 0.0;
	float VictoryElapsedTime = 0.f;
	double VictorySlowMotionStartTime = 0.0;

	void FinishSpawning();
	void CheckVictory();
	void FinishVictorySlowMotion();
};




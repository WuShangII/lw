// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class UBoxComponent;
class AEnemyBase;

DECLARE_MULTICAST_DELEGATE(FOnSpawnedEnemyCountChanged);

USTRUCT(BlueprintType)
struct LW_API FEnemySpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawning")
	TSubclassOf<AEnemyBase> EnemyClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawning", meta=(ClampMin="0.0"))
	float Weight = 1.f;

	bool IsSelectable() const;
};

USTRUCT(BlueprintType)
struct LW_API FEnemySpawnPhase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Phase", meta=(ClampMin="0.0", Units="s"))
    float StartTime = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Phase", meta=(ClampMin="0.1", Units="s"))
    float SpawnInterval = 3.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Phase", meta=(ClampMin="1", ClampMax="100"))
    int32 EnemiesPerBatch = 2;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Phase", meta=(ClampMin="0"))
    int32 MaxAliveEnemies = 20;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Phase", meta=(TitleProperty="EnemyClass"))
    TArray<FEnemySpawnEntry> Entries;
};

UCLASS()
class LW_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemySpawner();

	FOnSpawnedEnemyCountChanged OnEnemyCountChanged;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	AEnemyBase* SpawnOneEnemy();

	UFUNCTION(BlueprintCallable, Category="Spawning")
	void StartSpawning();
	UFUNCTION(BlueprintCallable, Category="Spawning")
	void StopSpawning();
	UFUNCTION(BlueprintCallable, Category="Spawning")
	void FinishSpawning();
	UFUNCTION(BlueprintCallable, Category="Spawning")
	void SetAutoSpawnEnabled(bool bEnabled);
	UFUNCTION(BlueprintPure, Category="Spawning")
	int32 GetAliveEnemyCount() const;
    UFUNCTION(BlueprintPure, Category="Spawning|Phases")
    float GetSpawningElapsedTime() const;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Spawning")
	bool bSpawningFinished = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawning|Phases", meta=(TitleProperty="StartTime"))
    TArray<FEnemySpawnPhase> SpawnPhases;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Spawning|Phases")
    int32 CurrentPhaseIndex = INDEX_NONE;
    FTimerHandle PhaseTimerHandle;
    double AccumulatedSpawnTime = 0.0;
    double SpawnClockStart = 0.0;
    bool bSpawnClockRunning = false;
    void UpdateSpawnPhase();
    int32 GetCurrentMaxAliveEnemies() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawning|Auto")
	bool bAutoSpawnEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawning|Auto", meta=(ClampMin="0.0", Units="s"))
	float InitialSpawnDelay = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawning|Manual", meta=(ClampMin="0"))
	int32 ManualSpawnLimit = 20;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AEnemyBase>> SpawnedEnemies;
	FTimerHandle SpawnTimerHandle;
	void SpawnBatch();
	UFUNCTION()
	void OnSpawnedEnemyDestroyed(AActor* DestroyedActor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	TObjectPtr<UBoxComponent> SpawnArea;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning|Manual")
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0.0", Units = "cm"))
	float MinSpawnDistance = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "1", ClampMax = "100"))
	int32 MaxSpawnAttempts = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "1.0", Units = "cm"))
	FVector NavigationQueryExtent = FVector(100.f, 100.f, 500.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0.0", Units = "cm"))
	float SpawnHeightOffset = 5.f;

	AEnemyBase* SpawnEnemyOfClass(TSubclassOf<AEnemyBase> ClassToSpawn);
	TSubclassOf<AEnemyBase> SelectAutoEnemyClass() const;
	bool FindSpawnLocation(TSubclassOf<AEnemyBase> ClassToSpawn, FVector& OutLocation) const;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

};

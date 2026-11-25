#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerUpgradeComponent.generated.h"

class UUpgradeDefinition;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUpgradeChoicesReady);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LW_API UPlayerUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UPlayerUpgradeComponent();
	UPROPERTY(BlueprintAssignable, Category="Upgrades")
	FOnUpgradeChoicesReady OnChoicesReady;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Upgrades")
	int32 PendingChoices = 0;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Upgrades")
	TArray<TObjectPtr<UUpgradeDefinition>> CurrentChoices;
	UFUNCTION(BlueprintPure, Category="Upgrades")
	int32 GetUpgradeStacks(FName UpgradeId) const;
	bool ChooseUpgrade(UUpgradeDefinition* Upgrade, int32 Revision);
	void ProcessPendingChoices();
	int32 GetChoiceRevision() const { return ChoiceRevision; }
	void CancelChoices();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Upgrades")
	TArray<TObjectPtr<UUpgradeDefinition>> UpgradePool;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Upgrades")
	TMap<FName, int32> SelectedStacks;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	UFUNCTION()
	void HandleLevelsGained(int32 LevelsGained);
	void GenerateChoices();
	int32 ChoiceRevision = 0;
	FTimerHandle ChoiceTimer;
};

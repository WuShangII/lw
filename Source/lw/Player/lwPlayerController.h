// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "lwPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class UDamageNumberLayerWidget;
class UUpgradeSelectionWidget;
class UPlayerUpgradeComponent;
class UUpgradeDefinition;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AlwPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void ShowDamageNumber(float Amount, const FVector& WorldLocation, bool bBurn);
	void ShowUpgradeMenu(UPlayerUpgradeComponent* Upgrades);
	void SelectUpgrade(UUpgradeDefinition* Upgrade, int32 Revision);
	void CloseUpgradeMenu(bool bResume);
	UFUNCTION(BlueprintCallable, Category="UI|Pause")
	void ShowPauseMenu();
	UFUNCTION(BlueprintCallable, Category="UI|Pause")
	void ResumeGame();

	bool GetMouseAimLocation(float GroundHeight, FVector& OutLocation) const;

	UFUNCTION(BlueprintCallable, Category="UI|Victory")
	void ShowVictoryScreen();

	UFUNCTION(BlueprintCallable, Category="UI|Victory")
	void RestartCurrentLevel();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="UI|Damage Numbers")
	TSubclassOf<UDamageNumberLayerWidget> DamageNumberLayerClass;
	UPROPERTY(Transient)
	TObjectPtr<UDamageNumberLayerWidget> DamageNumberLayer;
	UPROPERTY(EditDefaultsOnly, Category="UI|Upgrades")
	TSubclassOf<UUpgradeSelectionWidget> UpgradeWidgetClass;
	UPROPERTY(Transient) TObjectPtr<UUpgradeSelectionWidget> UpgradeWidget;
	TWeakObjectPtr<UPlayerUpgradeComponent> ActiveUpgrades;
	bool bUpgradeMenuOpen = false;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> PauseAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Pause")
	TSubclassOf<UUserWidget> PauseWidgetClass;
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PauseWidget;
	bool bPauseMenuOpen = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Victory")
	TSubclassOf<UUserWidget> VictoryWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> VictoryWidget;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

private:
	void FocusMenu(UUserWidget* Menu);
	void SetGameplayInputMode();

};

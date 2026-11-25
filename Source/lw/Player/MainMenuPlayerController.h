// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

class UUserWidget;

/**
 * 
 */
UCLASS()
class LW_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Menu")
	void StartGame();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Menu")
	TSubclassOf<UUserWidget> MenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Menu")
	TSoftObjectPtr<UWorld> GameLevel;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> MenuWidget;
};

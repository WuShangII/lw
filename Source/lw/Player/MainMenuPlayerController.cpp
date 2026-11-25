// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "lw.h"

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalPlayerController()) return;
	if (!MenuWidgetClass)
	{
		UE_LOG(Loglw, Error, TEXT("Configure MenuWidgetClass on the main menu controller."));
		return;
	}
	MenuWidget = CreateWidget<UUserWidget>(this, MenuWidgetClass);
	if (!MenuWidget) return;
	MenuWidget->AddToPlayerScreen(100);
	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(MenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AMainMenuPlayerController::StartGame()
{
	if (!IsLocalPlayerController()) return;
	if (GameLevel.IsNull())
	{
		UE_LOG(Loglw, Error, TEXT("Configure GameLevel on the main menu controller."));
		return;
	}
	if (MenuWidget)
	{
		MenuWidget->RemoveFromParent();
		MenuWidget = nullptr;
	}
	FlushPressedKeys();
	SetInputMode(FInputModeGameOnly());
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, GameLevel);
}


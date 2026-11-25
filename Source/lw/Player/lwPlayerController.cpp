// Copyright Epic Games, Inc. All Rights Reserved.


#include "lwPlayerController.h"
#include "Widgets/DamageNumberLayerWidget.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Others/lwGameMode.h"
#include "Components/PlayerUpgradeComponent.h"
#include "Widgets/UpgradeSelectionWidget.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "lw.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AlwPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() && !ShouldUseTouchControls())
	{
		SetGameplayInputMode();
		UWidgetBlueprintLibrary::SetFocusToGameViewport();
	}

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(Loglw, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AlwPlayerController::FocusMenu(UUserWidget* Menu)
{
	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Menu->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AlwPlayerController::SetGameplayInputMode()
{
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = !ShouldUseTouchControls();
}

void AlwPlayerController::ShowVictoryScreen()
{
	if (!IsLocalPlayerController())
	{
		return;
	}
	if (!VictoryWidgetClass)
	{
		UE_LOG(Loglw, Warning, TEXT("VictoryWidgetClass is not configured on the player controller."));
		return;
	}
	if (!VictoryWidget)
	{
		VictoryWidget = CreateWidget<UUserWidget>(this, VictoryWidgetClass);
	}
	if (!VictoryWidget)
	{
		return;
	}
	if (!VictoryWidget->IsInViewport())
	{
		VictoryWidget->AddToPlayerScreen(100);
	}
	FocusMenu(VictoryWidget);
}

void AlwPlayerController::RestartCurrentLevel()
{
	if (!IsLocalPlayerController())
	{
		return;
	}
	const FName LevelName(*UGameplayStatics::GetCurrentLevelName(this, true));
	if (VictoryWidget)
	{
		VictoryWidget->RemoveFromParent();
		VictoryWidget = nullptr;
	}
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
	UGameplayStatics::SetGamePaused(this, false);
	SetGameplayInputMode();
	UGameplayStatics::OpenLevel(this, LevelName);
}

bool AlwPlayerController::GetMouseAimLocation(float GroundHeight, FVector& OutLocation) const
{
	FVector RayOrigin;
	FVector RayDirection;
	if (!DeprojectMousePositionToWorld(RayOrigin, RayDirection) || FMath::IsNearlyZero(RayDirection.Z))
	{
		return false;
	}

	// 取鼠标射线与角色脚底水平面的交点。
	const double Distance = (GroundHeight - RayOrigin.Z) / RayDirection.Z;
	if (Distance < 0.0)
	{
		return false;
	}

	OutLocation = RayOrigin + RayDirection * Distance;
	return true;
}

void AlwPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseAction)
		{
			EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &AlwPlayerController::ShowPauseMenu);
		}
	}

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool AlwPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}


void AlwPlayerController::ShowPauseMenu()
{
	const AlwGameMode* Mode = GetWorld()->GetAuthGameMode<AlwGameMode>();
	if (!IsLocalPlayerController() || bPauseMenuOpen || bUpgradeMenuOpen || IsPaused() || !Mode || !Mode->CanPauseMatch()) return;
	if (!PauseWidgetClass)
	{
		UE_LOG(Loglw, Warning, TEXT("Configure PauseWidgetClass on the player controller."));
		return;
	}
	if (!PauseWidget) PauseWidget = CreateWidget<UUserWidget>(this, PauseWidgetClass);
	if (!PauseWidget || !PauseWidget->AddToPlayerScreen(100)) return;
	if (!SetPause(true))
	{
		PauseWidget->RemoveFromParent();
		return;
	}
	bPauseMenuOpen = true;
	FocusMenu(PauseWidget);
}

void AlwPlayerController::ResumeGame()
{
	const AlwGameMode* Mode = GetWorld()->GetAuthGameMode<AlwGameMode>();
	if (bUpgradeMenuOpen || !bPauseMenuOpen || !Mode || !Mode->CanPauseMatch()) return;
	if (!SetPause(false)) return;
	bPauseMenuOpen = false;
	if (PauseWidget) PauseWidget->RemoveFromParent();
	FlushPressedKeys();
	SetGameplayInputMode();
	if (APawn* PlayerPawn = GetPawn())
	{
		if (UPlayerUpgradeComponent* Upgrades = PlayerPawn->FindComponentByClass<UPlayerUpgradeComponent>())
		{
			Upgrades->ProcessPendingChoices();
		}
	}
}

void AlwPlayerController::ShowUpgradeMenu(UPlayerUpgradeComponent* Upgrades)
{
	const AlwGameMode* Mode = GetWorld()->GetAuthGameMode<AlwGameMode>();
	if (!IsLocalPlayerController() || !Mode || !Mode->CanPauseMatch() || bPauseMenuOpen
		|| (IsPaused() && !bUpgradeMenuOpen)) return;
	if (!UpgradeWidgetClass)
	{
		UE_LOG(Loglw, Error, TEXT("Configure UpgradeWidgetClass on the player controller."));
		return;
	}
	if (!UpgradeWidget) UpgradeWidget = CreateWidget<UUpgradeSelectionWidget>(this, UpgradeWidgetClass);
	if (!UpgradeWidget || !UpgradeWidget->RefreshChoices(Upgrades))
	{
		UE_LOG(Loglw, Error, TEXT("Cannot build upgrade cards. Check CardWidgetClass."));
		CloseUpgradeMenu(true);
		return;
	}
	if (!UpgradeWidget->IsInViewport() && !UpgradeWidget->AddToPlayerScreen(150)) return;
	if (!bUpgradeMenuOpen && !SetPause(true))
	{
		UpgradeWidget->RemoveFromParent();
		return;
	}
	ActiveUpgrades = Upgrades;
	bUpgradeMenuOpen = true;
	FocusMenu(UpgradeWidget);
}

void AlwPlayerController::SelectUpgrade(UUpgradeDefinition* Upgrade, int32 Revision)
{
	if (!bUpgradeMenuOpen || !ActiveUpgrades.IsValid()) return;
	if (!ActiveUpgrades->ChooseUpgrade(Upgrade, Revision)) return;
	if (ActiveUpgrades->PendingChoices > 0 && !ActiveUpgrades->CurrentChoices.IsEmpty())
	{
		ShowUpgradeMenu(ActiveUpgrades.Get());
	}
	else CloseUpgradeMenu(true);
}

void AlwPlayerController::CloseUpgradeMenu(bool bResume)
{
	if (!bResume && ActiveUpgrades.IsValid()) ActiveUpgrades->CancelChoices();
	if (UpgradeWidget) UpgradeWidget->RemoveFromParent();
	const bool bWasOpen = bUpgradeMenuOpen;
	bUpgradeMenuOpen = false;
	ActiveUpgrades.Reset();
	const AlwGameMode* Mode = GetWorld()->GetAuthGameMode<AlwGameMode>();
	const bool bReturnToGameplay = bResume && Mode && Mode->CanPauseMatch();
	if (bWasOpen && (!bResume || bReturnToGameplay))
	{
		SetPause(false);
		FlushPressedKeys();
		SetGameplayInputMode();
		UWidgetBlueprintLibrary::SetFocusToGameViewport();
	}
}

void AlwPlayerController::ShowDamageNumber(float Amount, const FVector& WorldLocation, bool bBurn)
{
    if (!IsLocalPlayerController() || !DamageNumberLayerClass) return;
    if (!DamageNumberLayer)
    {
        DamageNumberLayer = CreateWidget<UDamageNumberLayerWidget>(this, DamageNumberLayerClass);
        if (!DamageNumberLayer) return;
        DamageNumberLayer->AddToPlayerScreen(10);
        DamageNumberLayer->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    DamageNumberLayer->ShowDamage(Amount, WorldLocation, bBurn);
}

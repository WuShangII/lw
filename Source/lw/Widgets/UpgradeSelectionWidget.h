#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeSelectionWidget.generated.h"
class UHorizontalBox;
class UUpgradeCardWidget;
class UPlayerUpgradeComponent;
UCLASS()
class LW_API UUpgradeSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	bool RefreshChoices(UPlayerUpgradeComponent* Upgrades);
protected:
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UHorizontalBox> CardsBox;
	UPROPERTY(EditDefaultsOnly, Category="Upgrades") TSubclassOf<UUpgradeCardWidget> CardWidgetClass;
};

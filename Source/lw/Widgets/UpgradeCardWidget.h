#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeCardWidget.generated.h"
class UButton;
class UImage;
class UTextBlock;
class UUpgradeDefinition;
UCLASS()
class LW_API UUpgradeCardWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetUpgrade(UUpgradeDefinition* Definition, int32 Stacks, int32 Revision);
protected:
	virtual void NativeOnInitialized() override;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> SelectButton;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UImage> UpgradeIcon;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> UpgradeNameText;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> UpgradeDescriptionText;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> UpgradeStacksText;
private:
	UPROPERTY(Transient) TObjectPtr<UUpgradeDefinition> Upgrade;
	int32 ChoiceRevision = 0;
	UFUNCTION() void HandleClicked();
};

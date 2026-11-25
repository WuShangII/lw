#include "UpgradeCardWidget.h"
#include "Others/UpgradeDefinition.h"
#include "Player/lwPlayerController.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UUpgradeCardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SelectButton->OnClicked.AddDynamic(this, &UUpgradeCardWidget::HandleClicked);
}
void UUpgradeCardWidget::SetUpgrade(UUpgradeDefinition* Definition, int32 Stacks, int32 Revision)
{
	Upgrade = Definition;
	ChoiceRevision = Revision;
	UpgradeIcon->SetBrushFromTexture(Definition->Icon);
	UpgradeNameText->SetText(Definition->DisplayName);
	UpgradeDescriptionText->SetText(Definition->Description);
	UpgradeStacksText->SetText(FText::Format(FText::FromString(TEXT("{0} / {1}")), FText::AsNumber(Stacks), FText::AsNumber(Definition->MaxStacks)));
}
void UUpgradeCardWidget::HandleClicked()
{
	if (AlwPlayerController* PC = Cast<AlwPlayerController>(GetOwningPlayer()))
	{
		PC->SelectUpgrade(Upgrade, ChoiceRevision);
	}
}

#include "UpgradeSelectionWidget.h"
#include "UpgradeCardWidget.h"
#include "Others/UpgradeDefinition.h"
#include "Components/PlayerUpgradeComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

bool UUpgradeSelectionWidget::RefreshChoices(UPlayerUpgradeComponent* Upgrades)
{
	CardsBox->ClearChildren();
	if (!CardWidgetClass) return false;
	for (UUpgradeDefinition* Definition : Upgrades->CurrentChoices)
	{
		UUpgradeCardWidget* Card = CreateWidget<UUpgradeCardWidget>(GetOwningPlayer(), CardWidgetClass);
		if (!Card) return false;
		Card->SetUpgrade(Definition, Upgrades->GetUpgradeStacks(Definition->UpgradeId), Upgrades->GetChoiceRevision());
		UHorizontalBoxSlot* CardSlot = CardsBox->AddChildToHorizontalBox(Card);
		CardSlot->SetPadding(FMargin(12.f, 0.f));
		CardSlot->SetVerticalAlignment(VAlign_Center);
	}
	return CardsBox->GetChildrenCount() > 0;
}

#include "DamageNumberWidget.h"
#include "Components/TextBlock.h"

void UDamageNumberWidget::SetDamage(float Amount, bool bBurn)
{
    FNumberFormattingOptions Format;
    Format.SetUseGrouping(false);
    Format.SetMinimumFractionalDigits(0);
    Format.SetMaximumFractionalDigits(1);
    DamageText->SetText(Amount < 0.1f ? FText::FromString(TEXT("<0.1")) : FText::AsNumber(Amount, &Format));
    DamageText->SetColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(
        bBurn ? FColor(255, 101, 79) : FColor(255, 240, 208))));
    FSlateFontInfo Font = DamageText->GetFont();
    Font.Size = bBurn ? 22 : 26;
    DamageText->SetFont(Font);
}

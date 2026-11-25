#include "DamageNumberLayerWidget.h"
#include "DamageNumberWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/World.h"

void UDamageNumberLayerWidget::ShowDamage(float Amount, const FVector& WorldLocation, bool bBurn)
{
    if (!NumberWidgetClass || !FMath::IsFinite(Amount) || Amount <= 0.f) return;
    if (ActiveNumbers.Num() >= FMath::Clamp(MaxNumbers, 1, 200)) RecycleNumber(0);
    UDamageNumberWidget* NumberWidget = nullptr;
    if (!FreeNumbers.IsEmpty())
    {
        NumberWidget = FreeNumbers.Pop();
    }
    else
    {
        NumberWidget = CreateWidget<UDamageNumberWidget>(GetOwningPlayer(), NumberWidgetClass);
        if (!NumberWidget) return;
        UCanvasPanelSlot* NumberSlot = NumbersCanvas->AddChildToCanvas(NumberWidget);
        NumberSlot->SetAutoSize(true);
        NumberSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        NumberWidget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
    }
    NumberWidget->SetDamage(Amount, bBurn);
    FActiveDamageNumber Number;
    Number.Widget = NumberWidget;
    Number.WorldLocation = WorldLocation;
    Number.Offset = FVector2D(FMath::FRandRange(bBurn ? -12.f : -18.f, bBurn ? 12.f : 18.f), 0.f);
    Number.StartTime = GetWorld()->GetTimeSeconds();
    Number.bBurn = bBurn;
    ActiveNumbers.Add(Number);
    UpdateNumber(ActiveNumbers.Last(), Number.StartTime);
}

void UDamageNumberLayerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    const double Now = GetWorld()->GetTimeSeconds();
    for (int32 Index = ActiveNumbers.Num() - 1; Index >= 0; --Index)
    {
        const FActiveDamageNumber& Number = ActiveNumbers[Index];
        if (Now - Number.StartTime >= (Number.bBurn ? 0.6 : 0.75)) RecycleNumber(Index);
        else UpdateNumber(ActiveNumbers[Index], Now);
    }
}

void UDamageNumberLayerWidget::UpdateNumber(FActiveDamageNumber& Number, double Now)
{
    FVector2D Position;
    if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), Number.WorldLocation, Position, true))
    {
        Number.Widget->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }
    Number.Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
    const float Age = static_cast<float>(Now - Number.StartTime);
    const float Progress = FMath::Clamp(Age / (Number.bBurn ? 0.6f : 0.75f), 0.f, 1.f);
    const float FadeStart = Number.bBurn ? 0.45f : 0.55f;
    const float Opacity = 1.f - FMath::Clamp((Progress - FadeStart) / (1.f - FadeStart), 0.f, 1.f);
    const float Scale = Number.bBurn ? 1.f : FMath::Lerp(1.2f, 1.f, FMath::Clamp(Age / 0.1f, 0.f, 1.f));
    Number.Widget->SetRenderOpacity(Opacity);
    Number.Widget->SetRenderScale(FVector2D(Scale));
    if (UCanvasPanelSlot* NumberSlot = Cast<UCanvasPanelSlot>(Number.Widget->Slot))
    {
        NumberSlot->SetPosition(Position + Number.Offset - FVector2D(0.f, Progress * (Number.bBurn ? 40.f : 55.f)));
    }
}

void UDamageNumberLayerWidget::RecycleNumber(int32 Index)
{
    UDamageNumberWidget* NumberWidget = ActiveNumbers[Index].Widget;
    NumberWidget->SetVisibility(ESlateVisibility::Collapsed);
    FreeNumbers.Add(NumberWidget);
    ActiveNumbers.RemoveAt(Index);
}

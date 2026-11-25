#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageNumberLayerWidget.generated.h"

class UCanvasPanel;
class UDamageNumberWidget;

USTRUCT()
struct FActiveDamageNumber
{
    GENERATED_BODY()
    UPROPERTY() TObjectPtr<UDamageNumberWidget> Widget;
    FVector WorldLocation = FVector::ZeroVector;
    FVector2D Offset = FVector2D::ZeroVector;
    double StartTime = 0.0;
    bool bBurn = false;
};

UCLASS()
class LW_API UDamageNumberLayerWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    void ShowDamage(float Amount, const FVector& WorldLocation, bool bBurn);
protected:
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCanvasPanel> NumbersCanvas;
    UPROPERTY(EditDefaultsOnly, Category="Damage Numbers")
    TSubclassOf<UDamageNumberWidget> NumberWidgetClass;
    UPROPERTY(EditDefaultsOnly, Category="Damage Numbers", meta=(ClampMin="1", ClampMax="200"))
    int32 MaxNumbers = 100;
private:
    UPROPERTY(Transient) TArray<FActiveDamageNumber> ActiveNumbers;
    UPROPERTY(Transient) TArray<TObjectPtr<UDamageNumberWidget>> FreeNumbers;
    void UpdateNumber(FActiveDamageNumber& Number, double Now);
    void RecycleNumber(int32 Index);
};

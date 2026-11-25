#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageNumberWidget.generated.h"
class UTextBlock;

UCLASS()
class LW_API UDamageNumberWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    void SetDamage(float Amount, bool bBurn);
protected:
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UTextBlock> DamageText;
};

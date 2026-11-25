#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradeDefinition.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class EUpgradeEffectType : uint8
{
	AttackSpeed,
	ProjectileDamage,
	ProjectileLifetime,
	ProjectileCount,
	PierceCount,
	SpreadAngle,
	Burn,
	MeleeDamage,
	MeleeSpeed,
	MaxHealth,
	MoveSpeed,
	Frost,
	Knockback,
	HealOnKill
};

USTRUCT(BlueprintType)
struct LW_API FUpgradeEffect
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EUpgradeEffectType Type = EUpgradeEffectType::AttackSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float Amount = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.1", EditCondition="Type == EUpgradeEffectType::Burn", EditConditionHides))
	float BurnDuration = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.1", EditCondition="Type == EUpgradeEffectType::Burn", EditConditionHides))
	float BurnPeriod = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.1", EditCondition="Type == EUpgradeEffectType::Frost", EditConditionHides))
	float FrostDuration = 2.f;
};

UCLASS(BlueprintType)
class LW_API UUpgradeDefinition : public UDataAsset
{
	GENERATED_BODY()
public:
	bool HasValidEffects() const;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrade")
	FName UpgradeId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrade")
	FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrade", meta=(MultiLine=true))
	FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrade")
	TObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrade", meta=(ClampMin="0.0"))
	float Weight = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrade", meta=(ClampMin="1"))
	int32 MaxStacks = 5;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrade", meta=(TitleProperty="Type"))
	TArray<FUpgradeEffect> Effects;
};

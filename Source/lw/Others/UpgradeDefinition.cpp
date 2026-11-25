#include "UpgradeDefinition.h"

bool UUpgradeDefinition::HasValidEffects() const
{
	if (Effects.IsEmpty()) return false;
	for (const FUpgradeEffect& Effect : Effects)
	{
		if (!FMath::IsFinite(Effect.Amount) || Effect.Amount <= 0.f) return false;
		if ((Effect.Type == EUpgradeEffectType::ProjectileCount || Effect.Type == EUpgradeEffectType::PierceCount)
			&& (Effect.Amount >= static_cast<float>(MAX_int32) || Effect.Amount != FMath::FloorToFloat(Effect.Amount))) return false;
		if (Effect.Type == EUpgradeEffectType::Frost && (!FMath::IsFinite(Effect.FrostDuration) || Effect.FrostDuration <= 0.f || Effect.Amount > 0.8f)) return false;
		if (Effect.Type == EUpgradeEffectType::Burn && (!FMath::IsFinite(Effect.BurnDuration)
			|| !FMath::IsFinite(Effect.BurnPeriod) || Effect.BurnDuration <= 0.f || Effect.BurnPeriod < 0.1f)) return false;
	}
	return true;
}

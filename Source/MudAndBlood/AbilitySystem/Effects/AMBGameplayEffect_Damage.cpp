#include "AbilitySystem/Effects/AMBGameplayEffect_Damage.h"
#include "AbilitySystem/AMBGameplayTags.h"
#include "AbilitySystem/Attributes/AMBCombatAttributeSet.h"

UAMBGameplayEffect_Damage::UAMBGameplayEffect_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute = UAMBCombatAttributeSet::GetHealthAttribute();
	DamageModifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCallerDamage;
	SetByCallerDamage.DataTag = TAG_Data_Damage;
	DamageModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerDamage);

	Modifiers.Add(DamageModifier);
}

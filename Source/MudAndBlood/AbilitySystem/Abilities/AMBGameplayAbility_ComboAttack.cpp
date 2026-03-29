#include "AbilitySystem/Abilities/AMBGameplayAbility_ComboAttack.h"

#include "AbilitySystem/AMBGameplayTags.h"
#include "Variant_Combat/Components/CombatAttackComponent.h"

UAMBGameplayAbility_ComboAttack::UAMBGameplayAbility_ComboAttack()
{
	AbilityInputTag = TAG_Input_Attack_Light;
	ActivationBlockedTags.AddTag(TAG_State_Attack_Charged_Active);

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_Ability_Attack_Combo);
	SetAssetTags(AssetTags);
}

void UAMBGameplayAbility_ComboAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UCombatAttackComponent* CombatAttackComponent = GetCombatAttackComponentFromActorInfo())
	{
		CombatAttackComponent->DoComboAttackStart();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

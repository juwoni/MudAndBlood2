#include "AbilitySystem/Abilities/AMBGameplayAbility_CombatAction.h"

#include "Characters/AMBGASCharacterBase.h"
#include "Variant_Combat/Components/CombatAttackComponent.h"

UAMBGameplayAbility_CombatAction::UAMBGameplayAbility_CombatAction()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

AAMBGASCharacterBase* UAMBGameplayAbility_CombatAction::GetCombatCharacterFromActorInfo() const
{
	return CurrentActorInfo ? Cast<AAMBGASCharacterBase>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
}

UCombatAttackComponent* UAMBGameplayAbility_CombatAction::GetCombatAttackComponentFromActorInfo() const
{
	if (AAMBGASCharacterBase* Character = GetCombatCharacterFromActorInfo())
	{
		return Character->GetCombatAttackComponent();
	}

	return nullptr;
}

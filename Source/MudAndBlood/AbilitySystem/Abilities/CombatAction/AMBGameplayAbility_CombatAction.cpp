#include "AMBGameplayAbility_CombatAction.h"

#include "AMBCharacter.h"
#include "Variant_Combat/Components/CombatAttackComponent.h"

UAMBGameplayAbility_CombatAction::UAMBGameplayAbility_CombatAction()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

AAMBCharacter* UAMBGameplayAbility_CombatAction::GetCombatCharacterFromActorInfo() const
{
	return CurrentActorInfo ? Cast<AAMBCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
}

UCombatAttackComponent* UAMBGameplayAbility_CombatAction::GetCombatAttackComponentFromActorInfo() const
{
	if (AAMBCharacter* Character = GetCombatCharacterFromActorInfo())
	{
		return Character->FindComponentByClass<UCombatAttackComponent>();
	}

	return nullptr;
}

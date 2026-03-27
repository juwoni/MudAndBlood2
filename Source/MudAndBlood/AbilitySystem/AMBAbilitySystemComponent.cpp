#include "AbilitySystem/AMBAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/AMBGameplayAbility.h"

bool UAMBAbilitySystemComponent::HasAbilityWithInputTag(FGameplayTag InputTag) const
{
	if (!InputTag.IsValid())
	{
		return false;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		const UAMBGameplayAbility* Ability = Cast<UAMBGameplayAbility>(AbilitySpec.Ability);
		if (Ability && Ability->DoesAbilityMatchInputTag(InputTag))
		{
			return true;
		}
	}

	return false;
}

bool UAMBAbilitySystemComponent::TryActivateAbilitiesByInputTag(FGameplayTag InputTag, bool bAllowRemoteActivation)
{
	if (!InputTag.IsValid())
	{
		return false;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		const UAMBGameplayAbility* Ability = Cast<UAMBGameplayAbility>(AbilitySpec.Ability);
		if (!Ability || !Ability->DoesAbilityMatchInputTag(InputTag))
		{
			continue;
		}

		if (TryActivateAbility(AbilitySpec.Handle, bAllowRemoteActivation))
		{
			return true;
		}
	}

	return false;
}

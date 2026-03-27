#include "AbilitySystem/Abilities/AMBGameplayAbility.h"

#include "AbilitySystemComponent.h"

UAMBGameplayAbility::UAMBGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UAMBGameplayAbility::DoesAbilityMatchInputTag(FGameplayTag InputTag) const
{
	return AbilityInputTag.IsValid() && AbilityInputTag.MatchesTagExact(InputTag);
}

bool UAMBGameplayAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (RequiredCombatStyleTags.IsEmpty())
	{
		return true;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	return AbilitySystemComponent && AbilitySystemComponent->HasAnyMatchingGameplayTags(RequiredCombatStyleTags);
}

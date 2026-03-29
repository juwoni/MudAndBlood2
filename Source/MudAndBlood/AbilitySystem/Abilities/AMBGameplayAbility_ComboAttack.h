#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AMBGameplayAbility_CombatAction.h"
#include "AMBGameplayAbility_ComboAttack.generated.h"

UCLASS()
class MUDANDBLOOD_API UAMBGameplayAbility_ComboAttack : public UAMBGameplayAbility_CombatAction
{
	GENERATED_BODY()

public:
	UAMBGameplayAbility_ComboAttack();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};

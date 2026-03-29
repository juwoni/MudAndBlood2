#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AMBGameplayAbility.h"
#include "AMBGameplayAbility_CombatAction.generated.h"

class AAMBCharacter;
class UCombatAttackComponent;

UCLASS(Abstract)
class MUDANDBLOOD_API UAMBGameplayAbility_CombatAction : public UAMBGameplayAbility
{
	GENERATED_BODY()

public:
	UAMBGameplayAbility_CombatAction();

protected:
	AAMBCharacter* GetCombatCharacterFromActorInfo() const;
	UCombatAttackComponent* GetCombatAttackComponentFromActorInfo() const;
};

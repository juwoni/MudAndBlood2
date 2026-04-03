#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AMBGameplayAbility.h"
#include "AMBGameplayAbility_CombatAction.generated.h"

class AAMBGASCharacterBase;
class UCombatAttackComponent;

UCLASS(Abstract)
class MUDANDBLOOD_API UAMBGameplayAbility_CombatAction : public UAMBGameplayAbility
{
	GENERATED_BODY()

public:
	UAMBGameplayAbility_CombatAction();

protected:
	AAMBGASCharacterBase* GetCombatCharacterFromActorInfo() const;
	UCombatAttackComponent* GetCombatAttackComponentFromActorInfo() const;
};

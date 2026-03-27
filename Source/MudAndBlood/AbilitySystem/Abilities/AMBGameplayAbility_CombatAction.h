#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AMBGameplayAbility.h"
#include "AMBGameplayAbility_CombatAction.generated.h"

class AAMBCharacter;

UENUM(BlueprintType)
enum class EAMBCombatAbilityAction : uint8
{
	ComboStart,
	ComboEnd,
	ChargedStart,
	ChargedEnd
};

UCLASS(Abstract)
class MUDANDBLOOD_API UAMBGameplayAbility_CombatAction : public UAMBGameplayAbility
{
	GENERATED_BODY()

public:
	UAMBGameplayAbility_CombatAction();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	EAMBCombatAbilityAction CombatAction = EAMBCombatAbilityAction::ComboStart;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	AAMBCharacter* GetCombatCharacterFromActorInfo() const;
};

UCLASS()
class MUDANDBLOOD_API UAMBGameplayAbility_ComboAttack : public UAMBGameplayAbility_CombatAction
{
	GENERATED_BODY()

public:
	UAMBGameplayAbility_ComboAttack();
};

UCLASS()
class MUDANDBLOOD_API UAMBGameplayAbility_ChargedAttackStart : public UAMBGameplayAbility_CombatAction
{
	GENERATED_BODY()

public:
	UAMBGameplayAbility_ChargedAttackStart();
};

UCLASS()
class MUDANDBLOOD_API UAMBGameplayAbility_ChargedAttackEnd : public UAMBGameplayAbility_CombatAction
{
	GENERATED_BODY()

public:
	UAMBGameplayAbility_ChargedAttackEnd();
};

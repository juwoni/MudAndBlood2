#include "AbilitySystem/Abilities/AMBGameplayAbility_CombatAction.h"

#include "AMBCharacter.h"
#include "AbilitySystem/AMBGameplayTags.h"
#include "Variant_Combat/Components/CombatAttackComponent.h"

UAMBGameplayAbility_CombatAction::UAMBGameplayAbility_CombatAction()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UAMBGameplayAbility_CombatAction::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AAMBCharacter* Character = GetCombatCharacterFromActorInfo();
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	switch (CombatAction)
	{
	case EAMBCombatAbilityAction::ComboStart:
		if (UCombatAttackComponent* CombatAttackComponent = Character->GetCombatAttackComponent())
		{
			CombatAttackComponent->DoComboAttackStart();
		}
		break;
	case EAMBCombatAbilityAction::ComboEnd:
		if (UCombatAttackComponent* CombatAttackComponent = Character->GetCombatAttackComponent())
		{
			CombatAttackComponent->DoComboAttackEnd();
		}
		break;
	case EAMBCombatAbilityAction::ChargedStart:
		if (UCombatAttackComponent* CombatAttackComponent = Character->GetCombatAttackComponent())
		{
			CombatAttackComponent->DoChargedAttackStart();
		}
		break;
	case EAMBCombatAbilityAction::ChargedEnd:
		if (UCombatAttackComponent* CombatAttackComponent = Character->GetCombatAttackComponent())
		{
			CombatAttackComponent->DoChargedAttackEnd();
		}
		break;
	default:
		break;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

AAMBCharacter* UAMBGameplayAbility_CombatAction::GetCombatCharacterFromActorInfo() const
{
	return CurrentActorInfo ? Cast<AAMBCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
}

UAMBGameplayAbility_ComboAttack::UAMBGameplayAbility_ComboAttack()
{
	AbilityInputTag = TAG_Input_Attack_Light;
	AbilityTags.AddTag(TAG_Ability_Attack_Combo);
	CombatAction = EAMBCombatAbilityAction::ComboStart;
}

UAMBGameplayAbility_ChargedAttackStart::UAMBGameplayAbility_ChargedAttackStart()
{
	AbilityInputTag = TAG_Input_Attack_Heavy_Start;
	AbilityTags.AddTag(TAG_Ability_Attack_Charged);
	CombatAction = EAMBCombatAbilityAction::ChargedStart;
}

UAMBGameplayAbility_ChargedAttackEnd::UAMBGameplayAbility_ChargedAttackEnd()
{
	AbilityInputTag = TAG_Input_Attack_Heavy_Release;
	AbilityTags.AddTag(TAG_Ability_Attack_Charged);
	CombatAction = EAMBCombatAbilityAction::ChargedEnd;
}

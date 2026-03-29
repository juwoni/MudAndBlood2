#include "AbilitySystem/Abilities/AMBGameplayAbility_ChargedAttack.h"

#include "AbilitySystem/AMBGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Variant_Combat/Components/CombatAttackComponent.h"

UAMBGameplayAbility_ChargedAttack::UAMBGameplayAbility_ChargedAttack()
{
	AbilityInputTag = TAG_Input_Attack_Heavy_Start;
	ActivationOwnedTags.AddTag(TAG_State_Attack_Charged_Active);
	ActivationBlockedTags.AddTag(TAG_State_Attack_Charged_Active);

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_Ability_Attack_Charged);
	SetAssetTags(AssetTags);
}

void UAMBGameplayAbility_ChargedAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UCombatAttackComponent* CombatAttackComponent = GetCombatAttackComponentFromActorInfo();
	if (!CombatAttackComponent || !CombatAttackComponent->PlayChargedAttackMontage())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveCombatAttackComponent = CombatAttackComponent;
	bReleaseRequested = false;
	bHasEnteredChargeLoop = false;
	AttackMontageEndedHandle = CombatAttackComponent->OnAttackMontageFinished.AddUObject(
		this,
		&UAMBGameplayAbility_ChargedAttack::HandleAttackMontageEnded);

	ChargedAttackCheckTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_Attack_Charged_Check,
		nullptr,
		false,
		true);
	if (ChargedAttackCheckTask)
	{
		ChargedAttackCheckTask->EventReceived.AddDynamic(this, &UAMBGameplayAbility_ChargedAttack::HandleChargedAttackCheckpoint);
		ChargedAttackCheckTask->ReadyForActivation();
	}

	ChargedAttackReleaseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_Attack_Charged_Release,
		nullptr,
		false,
		true);
	if (ChargedAttackReleaseTask)
	{
		ChargedAttackReleaseTask->EventReceived.AddDynamic(this, &UAMBGameplayAbility_ChargedAttack::HandleChargedAttackRelease);
		ChargedAttackReleaseTask->ReadyForActivation();
	}
}

void UAMBGameplayAbility_ChargedAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UnbindChargedAttackDelegates();

	if (bWasCancelled)
	{
		if (UCombatAttackComponent* CombatAttackComponent = ActiveCombatAttackComponent.Get())
		{
			CombatAttackComponent->StopChargedAttackMontage();
		}
	}

	ActiveCombatAttackComponent.Reset();
	bReleaseRequested = false;
	bHasEnteredChargeLoop = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAMBGameplayAbility_ChargedAttack::HandleChargedAttackCheckpoint(FGameplayEventData Payload)
{
	static_cast<void>(Payload);

	UCombatAttackComponent* CombatAttackComponent = ActiveCombatAttackComponent.Get();
	if (!CombatAttackComponent)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	bHasEnteredChargeLoop = true;
	CombatAttackComponent->AdvanceChargedAttack(!bReleaseRequested);
}

void UAMBGameplayAbility_ChargedAttack::HandleChargedAttackRelease(FGameplayEventData Payload)
{
	static_cast<void>(Payload);

	bReleaseRequested = true;

	if (bHasEnteredChargeLoop)
	{
		if (UCombatAttackComponent* CombatAttackComponent = ActiveCombatAttackComponent.Get())
		{
			CombatAttackComponent->AdvanceChargedAttack(false);
		}
	}
}

void UAMBGameplayAbility_ChargedAttack::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	static_cast<void>(Montage);
	static_cast<void>(bInterrupted);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAMBGameplayAbility_ChargedAttack::UnbindChargedAttackDelegates()
{
	if (ChargedAttackCheckTask)
	{
		ChargedAttackCheckTask->EndTask();
		ChargedAttackCheckTask = nullptr;
	}

	if (ChargedAttackReleaseTask)
	{
		ChargedAttackReleaseTask->EndTask();
		ChargedAttackReleaseTask = nullptr;
	}

	if (UCombatAttackComponent* CombatAttackComponent = ActiveCombatAttackComponent.Get())
	{
		if (AttackMontageEndedHandle.IsValid())
		{
			CombatAttackComponent->OnAttackMontageFinished.Remove(AttackMontageEndedHandle);
			AttackMontageEndedHandle.Reset();
		}
	}
}

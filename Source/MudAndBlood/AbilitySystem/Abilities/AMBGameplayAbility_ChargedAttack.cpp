#include "AMBGameplayAbility_ChargedAttack.h"
#include "AbilitySystem/Abilities/AMBGameplayAbility_ChargedAttack.h"

#include "AbilitySystem/AMBGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Variant_Combat/Components/CombatAttackComponent.h"

UAMBGameplayAbility_ChargedAttack::UAMBGameplayAbility_ChargedAttack()
{
	AbilityInputTag = TAG_Input_Attack_Heavy_Start;
	ActivationOwnedTags.AddTag(TAG_State_Attack_Charged_Active);
	ActivationBlockedTags.AddTag(TAG_State_Attack_Combo_Active);
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

	UnbindChargedAttackDelegates();
	ActiveCombatAttackComponent.Reset();
	ChargedAttackPhase = EAMBChargedAttackPhase::None;

	UCombatAttackComponent* CombatAttackComponent = GetCombatAttackComponentFromActorInfo();
	if (!CombatAttackComponent || !CombatAttackComponent->PlayChargedAttackMontage())
	{
		RequestAbilityEnd(true);
		return;
	}

	ActiveCombatAttackComponent = CombatAttackComponent;
	ChargedAttackPhase = EAMBChargedAttackPhase::Starting;
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

	if (!ChargedAttackCheckTask || !ChargedAttackReleaseTask)
	{
		RequestAbilityEnd(true);
	}
}

void UAMBGameplayAbility_ChargedAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (ChargedAttackPhase == EAMBChargedAttackPhase::Ending)
	{
		return;
	}

	ChargedAttackPhase = EAMBChargedAttackPhase::Ending;
	UCombatAttackComponent* CombatAttackComponent = ActiveCombatAttackComponent.Get();
	UnbindChargedAttackDelegates();

	if (bWasCancelled && CombatAttackComponent)
	{
		CombatAttackComponent->StopChargedAttackMontage();
	}

	ActiveCombatAttackComponent.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAMBGameplayAbility_ChargedAttack::HandleChargedAttackCheckpoint(FGameplayEventData Payload)
{
	static_cast<void>(Payload);

	UCombatAttackComponent* CombatAttackComponent = GetActiveCombatAttackComponent();
	if (!CombatAttackComponent)
	{
		RequestAbilityEnd(true);
		return;
	}

	switch (ChargedAttackPhase)
	{
	case EAMBChargedAttackPhase::Starting:
		ChargedAttackPhase = EAMBChargedAttackPhase::Charging;
		CombatAttackComponent->AdvanceChargedAttack(true);
		break;
	case EAMBChargedAttackPhase::ReleaseRequested:
		EnterFinishingPhase(*CombatAttackComponent);
		break;
	case EAMBChargedAttackPhase::Charging:
		CombatAttackComponent->AdvanceChargedAttack(true);
		break;
	case EAMBChargedAttackPhase::Finishing:
	case EAMBChargedAttackPhase::Ending:
	case EAMBChargedAttackPhase::None:
	default:
		break;
	}
}

void UAMBGameplayAbility_ChargedAttack::HandleChargedAttackRelease(FGameplayEventData Payload)
{
	static_cast<void>(Payload);

	UCombatAttackComponent* CombatAttackComponent = GetActiveCombatAttackComponent();
	if (!CombatAttackComponent)
	{
		RequestAbilityEnd(true);
		return;
	}

	switch (ChargedAttackPhase)
	{
	case EAMBChargedAttackPhase::Starting:
		ChargedAttackPhase = EAMBChargedAttackPhase::ReleaseRequested;
		break;
	case EAMBChargedAttackPhase::Charging:
		EnterFinishingPhase(*CombatAttackComponent);
		break;
	case EAMBChargedAttackPhase::ReleaseRequested:
	case EAMBChargedAttackPhase::Finishing:
	case EAMBChargedAttackPhase::Ending:
	case EAMBChargedAttackPhase::None:
	default:
		break;
	}
}

void UAMBGameplayAbility_ChargedAttack::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	static_cast<void>(Montage);

	RequestAbilityEnd(bInterrupted);
}

void UAMBGameplayAbility_ChargedAttack::RequestAbilityEnd(bool bWasCancelled)
{
	if (ChargedAttackPhase == EAMBChargedAttackPhase::Ending)
	{
		return;
	}

	if (bWasCancelled && CurrentActorInfo)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

void UAMBGameplayAbility_ChargedAttack::EnterFinishingPhase(UCombatAttackComponent& CombatAttackComponent)
{
	if (ChargedAttackPhase == EAMBChargedAttackPhase::Finishing ||
		ChargedAttackPhase == EAMBChargedAttackPhase::Ending)
	{
		return;
	}

	// Commit only when the held charge actually converts into the attacking section.
	// This avoids spending cost/cooldown for a hold that gets cancelled before a swing is released.
	if (!CurrentActorInfo || !CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		RequestAbilityEnd(true);
		return;
	}

	ChargedAttackPhase = EAMBChargedAttackPhase::Finishing;
	CombatAttackComponent.AdvanceChargedAttack(false);
}

UCombatAttackComponent* UAMBGameplayAbility_ChargedAttack::GetActiveCombatAttackComponent() const
{
	if (ChargedAttackPhase == EAMBChargedAttackPhase::Ending ||
		ChargedAttackPhase == EAMBChargedAttackPhase::None)
	{
		return nullptr;
	}

	return ActiveCombatAttackComponent.Get();
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
		}
	}

	AttackMontageEndedHandle.Reset();
}

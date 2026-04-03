#include "AbilitySystem/Abilities/AMBGameplayAbility_ComboAttack.h"

#include "AbilitySystem/AMBGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"
#include "Characters/AMBGASCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Variant_Combat/Data/AMBCombatStyleData.h"
#include "Variant_Combat/Components/CombatAttackComponent.h"

UAMBGameplayAbility_ComboAttack::UAMBGameplayAbility_ComboAttack()
{
	AbilityInputTag = TAG_Input_Attack_Light;
	ActivationOwnedTags.AddTag(TAG_State_Attack_Combo_Active);
	ActivationBlockedTags.AddTag(TAG_State_Attack_Combo_Active);
	ActivationBlockedTags.AddTag(TAG_State_Attack_Charged_Active);

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_Ability_Attack_Combo);
	SetAssetTags(AssetTags);
}

void UAMBGameplayAbility_ComboAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	EndComboTasks();
	CachedComboInputTime = -1.0f;
	CurrentComboIndex = 0;
	ComboAttackPhase = EAMBComboAttackPhase::None;

	AAMBGASCharacterBase* Character = GetCombatCharacterFromActorInfo();
	UCombatAttackComponent* CombatAttackComponent = GetCombatAttackComponentFromActorInfo();
	const UAMBCombatStyleData* CombatStyleData = Character ? Character->GetCurrentCombatStyle() : nullptr;
	if (!Character ||
		!CombatStyleData ||
		!CombatStyleData->ComboAttackMontage ||
		CombatStyleData->ComboSectionNames.IsEmpty())
	{
		RequestAbilityEnd(true);
		return;
	}

	if (!CurrentActorInfo || !CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		RequestAbilityEnd(true);
		return;
	}

	if (CombatAttackComponent)
	{
		CombatAttackComponent->NotifyEnemiesOfIncomingAttack();
	}

	ComboAttackPhase = EAMBComboAttackPhase::Active;
	ComboInputTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_Attack_Combo_Input,
		nullptr,
		false,
		true);
	if (ComboInputTask)
	{
		ComboInputTask->EventReceived.AddDynamic(this, &UAMBGameplayAbility_ComboAttack::HandleComboInputReceived);
		ComboInputTask->ReadyForActivation();
	}

	ComboCheckpointTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_Attack_Combo_Check,
		nullptr,
		false,
		true);
	if (ComboCheckpointTask)
	{
		ComboCheckpointTask->EventReceived.AddDynamic(this, &UAMBGameplayAbility_ComboAttack::HandleComboCheckpoint);
		ComboCheckpointTask->ReadyForActivation();
	}

	ComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		CombatStyleData->ComboAttackMontage,
		1.0f,
		CombatStyleData->ComboSectionNames[0],
		true);
	if (ComboMontageTask)
	{
		ComboMontageTask->OnCompleted.AddDynamic(this, &UAMBGameplayAbility_ComboAttack::HandleComboMontageCompleted);
		ComboMontageTask->OnBlendOut.AddDynamic(this, &UAMBGameplayAbility_ComboAttack::HandleComboMontageBlendOut);
		ComboMontageTask->OnInterrupted.AddDynamic(this, &UAMBGameplayAbility_ComboAttack::HandleComboMontageInterrupted);
		ComboMontageTask->OnCancelled.AddDynamic(this, &UAMBGameplayAbility_ComboAttack::HandleComboMontageCancelled);
		ComboMontageTask->ReadyForActivation();
	}

	if (!ComboInputTask || !ComboCheckpointTask || !ComboMontageTask)
	{
		RequestAbilityEnd(true);
	}
}

void UAMBGameplayAbility_ComboAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (ComboAttackPhase == EAMBComboAttackPhase::Ending)
	{
		return;
	}

	ComboAttackPhase = EAMBComboAttackPhase::Ending;
	EndComboTasks();

	CachedComboInputTime = -1.0f;
	CurrentComboIndex = 0;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAMBGameplayAbility_ComboAttack::HandleComboInputReceived(FGameplayEventData Payload)
{
	static_cast<void>(Payload);

	if (ComboAttackPhase != EAMBComboAttackPhase::Active)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		RequestAbilityEnd(true);
		return;
	}

	CachedComboInputTime = World->GetTimeSeconds();
}

void UAMBGameplayAbility_ComboAttack::HandleComboCheckpoint(FGameplayEventData Payload)
{
	static_cast<void>(Payload);

	const UAMBCombatStyleData* CombatStyleData = GetActiveCombatStyleData();
	if (!CombatStyleData)
	{
		RequestAbilityEnd(true);
		return;
	}

	TryAdvanceCombo(*CombatStyleData);
}

void UAMBGameplayAbility_ComboAttack::HandleComboMontageCompleted()
{
	RequestAbilityEnd(false);
}

void UAMBGameplayAbility_ComboAttack::HandleComboMontageBlendOut()
{
	RequestAbilityEnd(false);
}

void UAMBGameplayAbility_ComboAttack::HandleComboMontageInterrupted()
{
	RequestAbilityEnd(true);
}

void UAMBGameplayAbility_ComboAttack::HandleComboMontageCancelled()
{
	RequestAbilityEnd(true);
}

void UAMBGameplayAbility_ComboAttack::RequestAbilityEnd(bool bWasCancelled)
{
	if (ComboAttackPhase == EAMBComboAttackPhase::Ending)
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

bool UAMBGameplayAbility_ComboAttack::TryAdvanceCombo(const UAMBCombatStyleData& CombatStyleData)
{
	if (ComboAttackPhase != EAMBComboAttackPhase::Active || CachedComboInputTime < 0.0f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World || World->GetTimeSeconds() - CachedComboInputTime > CombatStyleData.ComboInputCacheTimeTolerance)
	{
		return false;
	}

	const int32 NextComboIndex = CurrentComboIndex + 1;
	if (!CombatStyleData.ComboSectionNames.IsValidIndex(NextComboIndex))
	{
		return false;
	}

	AAMBGASCharacterBase* Character = GetCombatCharacterFromActorInfo();
	UAnimInstance* AnimInstance = Character && Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		RequestAbilityEnd(true);
		return false;
	}

	CurrentComboIndex = NextComboIndex;
	CachedComboInputTime = -1.0f;
	AnimInstance->Montage_JumpToSection(CombatStyleData.ComboSectionNames[CurrentComboIndex], CombatStyleData.ComboAttackMontage);
	return true;
}

const UAMBCombatStyleData* UAMBGameplayAbility_ComboAttack::GetActiveCombatStyleData() const
{
	if (ComboAttackPhase == EAMBComboAttackPhase::Ending || ComboAttackPhase == EAMBComboAttackPhase::None)
	{
		return nullptr;
	}

	const AAMBGASCharacterBase* Character = GetCombatCharacterFromActorInfo();
	return Character ? Character->GetCurrentCombatStyle() : nullptr;
}

void UAMBGameplayAbility_ComboAttack::EndComboTasks()
{
	if (ComboInputTask)
	{
		ComboInputTask->EndTask();
		ComboInputTask = nullptr;
	}

	if (ComboCheckpointTask)
	{
		ComboCheckpointTask->EndTask();
		ComboCheckpointTask = nullptr;
	}

	if (ComboMontageTask)
	{
		ComboMontageTask->EndTask();
		ComboMontageTask = nullptr;
	}
}

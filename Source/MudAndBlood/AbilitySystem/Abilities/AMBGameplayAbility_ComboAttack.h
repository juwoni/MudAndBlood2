#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AMBGameplayAbility_CombatAction.h"
#include "AMBGameplayAbility_ComboAttack.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_PlayMontageAndWait;
class UAMBCombatStyleData;

enum class EAMBComboAttackPhase : uint8
{
	None,
	Active,
	Ending
};

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

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	UFUNCTION()
	void HandleComboInputReceived(FGameplayEventData Payload);

	UFUNCTION()
	void HandleComboCheckpoint(FGameplayEventData Payload);

	UFUNCTION()
	void HandleComboMontageCompleted();

	UFUNCTION()
	void HandleComboMontageBlendOut();

	UFUNCTION()
	void HandleComboMontageInterrupted();

	UFUNCTION()
	void HandleComboMontageCancelled();

	void RequestAbilityEnd(bool bWasCancelled);
	bool TryAdvanceCombo(const UAMBCombatStyleData& CombatStyleData);
	const UAMBCombatStyleData* GetActiveCombatStyleData() const;
	void EndComboTasks();

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ComboInputTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ComboCheckpointTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> ComboMontageTask;

	float CachedComboInputTime = -1.0f;
	int32 CurrentComboIndex = 0;
	EAMBComboAttackPhase ComboAttackPhase = EAMBComboAttackPhase::None;
};

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AMBGameplayAbility_CombatAction.h"
#include "AMBGameplayAbility_ChargedAttack.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;
class UCombatAttackComponent;

enum class EAMBChargedAttackPhase : uint8
{
	None,
	Starting,
	ReleaseRequested,
	Charging,
	Finishing,
	Ending
};

UCLASS()
class MUDANDBLOOD_API UAMBGameplayAbility_ChargedAttack : public UAMBGameplayAbility_CombatAction
{
	GENERATED_BODY()

public:
	UAMBGameplayAbility_ChargedAttack();

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
	void HandleChargedAttackCheckpoint(FGameplayEventData Payload);

	UFUNCTION()
	void HandleChargedAttackRelease(FGameplayEventData Payload);

	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void RequestAbilityEnd(bool bWasCancelled);
	void EnterFinishingPhase(UCombatAttackComponent& CombatAttackComponent);
	UCombatAttackComponent* GetActiveCombatAttackComponent() const;
	void UnbindChargedAttackDelegates();

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ChargedAttackCheckTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ChargedAttackReleaseTask;

	TWeakObjectPtr<UCombatAttackComponent> ActiveCombatAttackComponent;
	FDelegateHandle AttackMontageEndedHandle;
	EAMBChargedAttackPhase ChargedAttackPhase = EAMBChargedAttackPhase::None;
};

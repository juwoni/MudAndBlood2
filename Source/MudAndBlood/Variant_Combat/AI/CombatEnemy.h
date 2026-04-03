// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMontage.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "Characters/AMBGASCharacterBase.h"
#include "Engine/TimerHandle.h"
#include "CombatEnemy.generated.h"

class UWidgetComponent;
class UCombatLifeBar;
class UAnimMontage;
class UAMBCombatStyleData;
class UCombatAttackComponent;

DECLARE_DELEGATE(FOnEnemyAttackCompleted);
DECLARE_DELEGATE(FOnEnemyLanded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDied);

UCLASS(abstract)
class MUDANDBLOOD_API ACombatEnemy : public AAMBGASCharacterBase
{
	GENERATED_BODY()

	/** Life bar widget component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* LifeBar;

	/** Reusable attack component used for weapon sphere traces. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCombatAttackComponent* CombatAttackComponent;

public:
	ACombatEnemy();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Style")
	TObjectPtr<UAMBCombatStyleData> CombatStyle;

	/** Max amount of HP the character will have on respawn */
	UPROPERTY(EditAnywhere, Category="Damage")
	float MaxHP = 3.0f;

public:
	/** Mirror of current GAS health for BP/UI compatibility */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Damage", meta = (ClampMin = 0, ClampMax = 100))
	float CurrentHP = 0.0f;

protected:
	/** Name of the pelvis bone, for damage ragdoll physics */
	UPROPERTY(EditAnywhere, Category="Damage")
	FName PelvisBoneName;

	/** Pointer to the life bar widget */
	UPROPERTY(EditAnywhere, Category="Damage")
	UCombatLifeBar* LifeBarWidget;

	/** If true, the character is currently playing an attack animation */
	bool bIsAttacking = false;

	/** Distance ahead of the character that melee attack sphere collision traces will extend */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float MeleeTraceDistance = 75.0f;

	/** Radius of the sphere trace for melee attacks */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float MeleeTraceRadius = 50.0f;

	/** Amount of damage a melee attack will deal */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 100))
	float MeleeDamage = 1.0f;

	/** Amount of knockback impulse a melee attack will apply */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MeleeKnockbackImpulse = 100.0f;

	/** Amount of upwards impulse a melee attack will apply */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MeleeLaunchImpulse = 35.0f;

	/** Legacy fallback AnimMontage for combo attacks */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Combo")
	UAnimMontage* ComboAttackMontage = nullptr;

	/** Legacy fallback section names for combo attacks */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Combo")
	TArray<FName> ComboSectionNames;

	/** Target number of attacks in the combo attack string we're playing */
	int32 TargetComboCount = 0;

	/** Index of the current stage of the melee attack combo */
	int32 CurrentComboAttack = 0;

	/** Legacy fallback AnimMontage for charged attacks */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Charged")
	UAnimMontage* ChargedAttackMontage = nullptr;

	/** Legacy fallback loop section name */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Charged")
	FName ChargeLoopSection;

	/** Legacy fallback attack section name */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Charged")
	FName ChargeAttackSection;

	/** Minimum number of charge animation loops that will be played by the AI */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Charged", meta = (ClampMin = 1, ClampMax = 20))
	int32 MinChargeLoops = 2;

	/** Maximum number of charge animation loops that will be played by the AI */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Charged", meta = (ClampMin = 1, ClampMax = 20))
	int32 MaxChargeLoops = 5;

	/** Target number of charge animation loops to play in this charged attack */
	int32 TargetChargeLoops = 0;

	/** Number of charge animation loops currently played */
	int32 CurrentChargeLoop = 0;

	/** Time to wait before removing this character from the level after it dies */
	UPROPERTY(EditAnywhere, Category="Death")
	float DeathRemovalTime = 5.0f;

	/** Enemy death timer */
	FTimerHandle DeathTimer;

	/** Last recorded location we're being attacked from */
	FVector LastDangerLocation = FVector::ZeroVector;

	/** Last recorded game time we were attacked */
	float LastDangerTime = -1000.0f;

	UPROPERTY(Transient)
	TObjectPtr<UAMBCombatStyleData> CurrentCombatStyle;

	FGameplayTag CurrentCombatStyleTag;
	TArray<FGameplayAbilitySpecHandle> GrantedCombatAbilityHandles;
	FDelegateHandle ComboStateTagChangedHandle;
	FDelegateHandle ChargedStateTagChangedHandle;

public:
	FOnEnemyAttackCompleted OnAttackCompleted;
	FOnEnemyLanded OnEnemyLanded;

	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnEnemyDied OnEnemyDied;

public:
	void DoAIComboAttack();
	void DoAIChargedAttack();

	const FVector& GetLastDangerLocation() const;
	float GetLastDangerTime() const;

	virtual UCombatAttackComponent* GetCombatAttackComponent() const override { return CombatAttackComponent; }
	virtual UAMBCombatStyleData* GetCurrentCombatStyle() const override { return CurrentCombatStyle; }

	UFUNCTION(BlueprintCallable, Category="Attacker")
	virtual void CheckCombo() override;

	UFUNCTION(BlueprintCallable, Category="Attacker")
	virtual void CheckChargedAttack() override;

	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;
	virtual void HandleDeath() override;
	virtual void ApplyHealing(float Healing, AActor* Healer) override;
	virtual void NotifyDanger(const FVector& DangerLocation, AActor* DangerSource) override;

protected:
	virtual void PrepareAttackTrace() override;
	virtual void HandleHealthChanged(float OldHealth, float NewHealth, AActor* InstigatorActor) override;

	void RemoveFromLevel();

public:
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Landed(const FHitResult& Hit) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void ReceivedDamage(float Damage, const FVector& ImpactPoint, const FVector& DamageDirection);

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	void ClearGrantedCombatAbilities();
	void GrantCombatStyleAbilities(const UAMBCombatStyleData* CombatStyleData);
	void UpdateCombatStyleTag(const FGameplayTag& NewCombatStyleTag);
	bool TryActivateCombatAbilityByInputTag(const FGameplayTag& InputTag) const;
	void SetCombatStyle(UAMBCombatStyleData* NewCombatStyle);
	void HandleComboAbilityStateChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void HandleChargedAbilityStateChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void NotifyAttackCompleted();
};

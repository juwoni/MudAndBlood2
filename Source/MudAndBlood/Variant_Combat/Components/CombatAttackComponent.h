// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "CombatAttackComponent.generated.h"

class ACharacter;
class UAnimMontage;
class UAMBCombatStyleData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombatAttackDamageDealtSignature, float, Damage, FVector, ImpactPoint);

DECLARE_MULTICAST_DELEGATE_TwoParams(FCombatAttackMontageEndedSignature, UAnimMontage*, bool);

/**
 * Reusable melee attack logic extracted from ACombatCharacter.
 * Intended to be owned by a Character and driven by input or animation notifies.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class MUDANDBLOOD_API UCombatAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatAttackComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace")
	bool bWeaponTrace;

	void ApplyWeaponDamage(AActor* HitActor, const FVector& ImpactPoint);
	void SetWeaponTrace(bool isTracing);

	void SetMeleeTraceSettings(float TraceDistance, float TraceRadius, float Damage, float KnockbackImpulse,
	                           float LaunchImpulse);

	/** Handles combo attack pressed from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Combat|Input")
	virtual void DoComboAttackStart();

	/** Handles combo attack released from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Combat|Input")
	virtual void DoComboAttackEnd();

	/** Performs a box trace for weapon-based attacks and can be overridden in Blueprint. */
	UFUNCTION(BlueprintCallable, Category="Combat|Attack")
	virtual bool AttackBoxTrace(FHitResult& OutHitResult);
	FVector PreTopSocket = FVector::ZeroVector;
	FVector PreBottomSocket = FVector::ZeroVector;

	/** Performs the collision check for an attack */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Combat|Attack")
	void DoAttackTrace(FName TraceStartBone, FName TraceEndBone);
	virtual void DoAttackTrace_Implementation(FName TraceStartBone, FName TraceEndBone);

	/** Performs a sphere trace for attack targets and returns the first valid hit for BP usage. */
	UFUNCTION(BlueprintCallable, Category="Combat|Attack")
	bool AttackSphereTrace(FName TraceStartBone, FName TraceEndBone, FHitResult& OutHitResult);

	/** Starts a continuous attack trace window. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Combat|Attack")
	void BeginAttackTraceWindow(FName TraceStartBone, FName TraceEndBone);
	virtual void BeginAttackTraceWindow_Implementation(FName TraceStartBone, FName TraceEndBone);

	/** Updates a continuous attack trace window. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Combat|Attack")
	void TickAttackTraceWindow(FName TraceStartBone, FName TraceEndBone);
	virtual void TickAttackTraceWindow_Implementation(FName TraceStartBone, FName TraceEndBone);

	/** Ends a continuous attack trace window. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Combat|Attack")
	void EndAttackTraceWindow();
	virtual void EndAttackTraceWindow_Implementation();

	/** Performs the combo string check */
	UFUNCTION(BlueprintCallable, Category="Combat|Attack")
	virtual void CheckCombo();

	/** Notifies nearby enemies that an attack is coming so they can react */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Combat|Attack")
	void NotifyEnemiesOfIncomingAttack();
	virtual void NotifyEnemiesOfIncomingAttack_Implementation();

	/** Applies a style data asset so the same component can be reused by multiple weapon types */
	UFUNCTION(BlueprintCallable, Category="Combat|Style")
	void ApplyCombatStyleData(UAMBCombatStyleData* CombatStyleData);

	/** Broadcast after successfully dealing damage so the owner can play VFX/SFX */
	UPROPERTY(BlueprintAssignable, Category="Combat|Attack")
	FCombatAttackDamageDealtSignature OnDamageDealt;

	FCombatAttackMontageEndedSignature OnAttackMontageFinished;

	UFUNCTION(BlueprintCallable, Category="Combat|Charged")
	bool PlayChargedAttackMontage();

	UFUNCTION(BlueprintCallable, Category="Combat|Charged")
	void AdvanceChargedAttack(bool bShouldLoopCharge);

	UFUNCTION(BlueprintCallable, Category="Combat|Charged")
	void StopChargedAttackMontage(float BlendOutTime = 0.1f);

protected:
	/** Max amount of time that may elapse for a non-combo attack input to not be considered stale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack",
		meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float AttackInputCacheTimeTolerance = 1.0f;

	/** Distance ahead of the character that melee attack sphere collision traces will extend */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace",
		meta = (ClampMin = 0, ClampMax = 500, Units="cm"))
	float MeleeTraceDistance = 75.0f;

	/** Radius of the sphere trace for melee attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace",
		meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
	float MeleeTraceRadius = 75.0f;

	/** Distance ahead of the character that enemies will be notified of incoming attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace",
		meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float DangerTraceDistance = 300.0f;

	/** Radius of the sphere trace to notify enemies of incoming attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace",
		meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
	float DangerTraceRadius = 100.0f;

	/** If true, draw debug geometry whenever incoming attack danger is notified. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace|Debug")
	bool bDrawDangerTraceDebug = false;

	/** How long the danger trace debug geometry should remain visible. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace|Debug",
		meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float DangerTraceDebugDuration = 1.0f;

	/** If true, draw debug geometry for the actual attack sweep used to deal damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace|Debug")
	bool bDrawWeaponDamageTraceDebug = false;

	/** How long the weapon damage trace debug geometry should remain visible. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace|Debug",
		meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float WeaponDamageTraceDebugDuration = 1.0f;

	/** Amount of damage a melee attack will deal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 100))
	float MeleeDamage = 1.0f;

	/** Amount of knockback impulse a melee attack will apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Damage",
		meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MeleeKnockbackImpulse = 250.0f;

	/** Amount of upwards impulse a melee attack will apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Damage",
		meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MeleeLaunchImpulse = 300.0f;

	/** AnimMontage that will play for combo attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Combo")
	TObjectPtr<UAnimMontage> ComboAttackMontage = nullptr;

	/** Names of the AnimMontage sections that correspond to each stage of the combo attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Combo")
	TArray<FName> ComboSectionNames;

	/** Max amount of time that may elapse for a combo attack input to not be considered stale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Combo",
		meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float ComboInputCacheTimeTolerance = 0.45f;

	/** AnimMontage that will play for charged attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Charged")
	TObjectPtr<UAnimMontage> ChargedAttackMontage = nullptr;

	/** Name of the AnimMontage section that corresponds to the charge loop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Charged")
	FName ChargeLoopSection;

	/** Name of the AnimMontage section that corresponds to the attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Charged")
	FName ChargeAttackSection;

	/** Performs a combo attack */
	void ComboAttack();

	/** Called from a delegate when the attack montage ends */
	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** Returns the character that owns this component */
	ACharacter* GetCharacterOwner() const;

	/** Returns the owner's anim instance, if available */
	UAnimInstance* GetOwnerAnimInstance() const;

private:
	/** Time at which an attack button was last pressed */
	float CachedAttackInputTime = 0.0f;

	/** If true, the owner is currently playing an attack animation */
	bool bIsAttacking = false;

	/** Index of the current stage of the melee attack combo */
	int32 ComboCount = 0;

	/** Attack montage ended delegate */
	FOnMontageEnded OnAttackMontageEnded;
};

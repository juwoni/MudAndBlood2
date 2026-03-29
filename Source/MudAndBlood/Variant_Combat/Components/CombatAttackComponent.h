// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetSystemLibrary.h"
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
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class MUDANDBLOOD_API UCombatAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatAttackComponent();

	/** Handles combo attack pressed from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Combat|Input")
	virtual void DoComboAttackStart();

	/** Handles combo attack released from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Combat|Input")
	virtual void DoComboAttackEnd();

	/** Performs the collision check for an attack */
	UFUNCTION(BlueprintCallable, Category="Combat|Attack")
	virtual void DoAttackTrace(FName TraceStartBone, FName TraceEndBone);

	/** Starts a continuous attack trace window. */
	UFUNCTION(BlueprintCallable, Category="Combat|Attack")
	virtual void BeginAttackTraceWindow(FName TraceStartBone, FName TraceEndBone);

	/** Updates a continuous attack trace window. */
	UFUNCTION(BlueprintCallable, Category="Combat|Attack")
	virtual void TickAttackTraceWindow(FName TraceStartBone, FName TraceEndBone);

	/** Ends a continuous attack trace window. */
	UFUNCTION(BlueprintCallable, Category="Combat|Attack")
	virtual void EndAttackTraceWindow();

	/** Performs the combo string check */
	UFUNCTION(BlueprintCallable, Category="Combat|Attack")
	virtual void CheckCombo();

	/** Notifies nearby enemies that an attack is coming so they can react */
	UFUNCTION(BlueprintCallable, Category="Combat|Attack")
	virtual void NotifyEnemiesOfIncomingAttack();

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float AttackInputCacheTimeTolerance = 1.0f;

	/** Distance ahead of the character that melee attack sphere collision traces will extend */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 500, Units="cm"))
	float MeleeTraceDistance = 75.0f;

	/** Radius of the sphere trace for melee attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
	float MeleeTraceRadius = 75.0f;

	/** Distance ahead of the character that enemies will be notified of incoming attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float DangerTraceDistance = 300.0f;

	/** Radius of the sphere trace to notify enemies of incoming attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
	float DangerTraceRadius = 100.0f;

	/** If true, draw debug geometry whenever incoming attack danger is notified. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace|Debug")
	bool bDrawDangerTraceDebug = false;

	/** How long the danger trace debug geometry should remain visible. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace|Debug", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float DangerTraceDebugDuration = 1.0f;

	/** If true, draw debug geometry for the actual attack sweep used to deal damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace|Debug")
	bool bDrawWeaponDamageTraceDebug = false;

	/** How long the weapon damage trace debug geometry should remain visible. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Trace|Debug", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float WeaponDamageTraceDebugDuration = 1.0f;

	/** Amount of damage a melee attack will deal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 100))
	float MeleeDamage = 1.0f;

	/** Amount of knockback impulse a melee attack will apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MeleeKnockbackImpulse = 250.0f;

	/** Amount of upwards impulse a melee attack will apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MeleeLaunchImpulse = 300.0f;

	/** AnimMontage that will play for combo attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Combo")
	TObjectPtr<UAnimMontage> ComboAttackMontage = nullptr;

	/** Names of the AnimMontage sections that correspond to each stage of the combo attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Combo")
	TArray<FName> ComboSectionNames;

	/** Max amount of time that may elapse for a combo attack input to not be considered stale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack|Combo", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
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

protected:
	UStaticMeshComponent* GetEquippedWeaponMesh(ACharacter* CharacterOwner) const;
	bool TryResolveAttackTraceLocation(ACharacter* CharacterOwner, FName SocketName, FVector& OutLocation) const;
	bool TryResolveAttackTracePoints(ACharacter* CharacterOwner, FName TraceStartBone, FName TraceEndBone, FVector& OutTraceStart, FVector& OutTraceEnd) const;
	void PerformAttackTraceSweep(ACharacter* CharacterOwner, const FVector& TraceStart, const FVector& TraceEnd, TSet<TWeakObjectPtr<AActor>>& AlreadyHitActors);
	FVector GetAttackTraceHalfSize(ACharacter* CharacterOwner) const;
	ETraceTypeQuery GetAttackTraceChannel() const;
	EDrawDebugTrace::Type GetAttackTraceDrawDebugType() const;
	float GetAttackTraceDebugLifetime(const UWorld* World) const;
	void ResetAttackTraceWindowState();

	static const FName DefaultWeaponAttackTraceStartSocketName;
	static const FName DefaultWeaponAttackTraceEndSocketName;

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

	/** If true, an AnimNotifyState currently owns the melee trace window. */
	bool bIsAttackTraceWindowActive = false;

	/** If true, previous frame trace points have been cached for the active attack window. */
	bool bHasPreviousAttackTracePoints = false;

	/** Previous frame start point used for continuous attack sweeps. */
	FVector PreviousAttackTraceStart = FVector::ZeroVector;

	/** Previous frame end point used for continuous attack sweeps. */
	FVector PreviousAttackTraceEnd = FVector::ZeroVector;

	/** Actors already damaged during the current continuous attack window. */
	TSet<TWeakObjectPtr<AActor>> DamagedActorsInTraceWindow;

	/** Attack montage ended delegate */
	FOnMontageEnded OnAttackMontageEnded;
};

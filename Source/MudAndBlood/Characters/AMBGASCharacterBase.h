#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "CombatAttacker.h"
#include "CombatDamageable.h"
#include "GameFramework/Character.h"
#include "AMBGASCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAMBAbilitySystemComponent;
class UAMBCombatAttributeSet;
class UAMBCombatStyleData;
class UCombatAttackComponent;
class UGameplayEffect;
struct FHitResult;

UCLASS(Blueprintable, BlueprintType)
class MUDANDBLOOD_API AAMBGASCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatAttacker, public ICombatDamageable
{
	GENERATED_BODY()

public:
	AAMBGASCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category="Combat|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category="Combat|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category="Combat|Attributes")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintCallable, Category="Combat|Damage")
	bool ApplyDamageToTarget(AActor* TargetActor, float DamageAmount, AActor* InstigatorActor = nullptr, UObject* SourceObject = nullptr);

	UFUNCTION(BlueprintCallable, Category="Combat|Damage")
	bool ApplyDamageToSelf(float DamageAmount, AActor* InstigatorActor = nullptr, UObject* SourceObject = nullptr);

	UFUNCTION(BlueprintCallable, Category="Combat|Healing")
	bool ApplyHealingToSelf(float HealingAmount, AActor* Healer = nullptr, UObject* SourceObject = nullptr);

	UFUNCTION(BlueprintImplementableEvent, Category="Combat|Damage", meta=(DisplayName="On GAS Damaged"))
	void BP_OnGASDamaged(float Damage, float CurrentHealth, FVector DamageLocation, FVector DamageDirection, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent, Category="Combat|Damage", meta=(DisplayName="On GAS Died"))
	void BP_OnGASDied(AActor* DamageCauser);

	virtual UCombatAttackComponent* GetCombatAttackComponent() const;
	virtual UAMBCombatStyleData* GetCurrentCombatStyle() const;

	virtual bool SphereTraceMultiForObjects(FName TraceStartBone, FName TraceEndBone, AActor*& HitActor, FVector& ImpactPoint) override;
	virtual void DoAttackTrace(FName TraceStartBone, FName TraceEndBone) override;
	virtual void BeginAttackTraceWindow(FName TraceStartBone, FName TraceEndBone) override;
	virtual void TickAttackTraceWindow(FName TraceStartBone, FName TraceEndBone) override;
	virtual void EndAttackTraceWindow() override;
	virtual void CheckCombo() override;
	virtual void CheckChargedAttack() override;

	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;
	virtual void HandleDeath() override;
	virtual void ApplyHealing(float Healing, AActor* Healer) override;
	virtual void NotifyDanger(const FVector& DangerLocation, AActor* DangerSource) override;

	UFUNCTION(BlueprintPure, Category="Combat|Attributes")
	UAMBAbilitySystemComponent* GetAMBAbilitySystemComponent() const { return AbilitySystemComponent; }

	UFUNCTION(BlueprintPure, Category="Combat|Attributes")
	UAMBCombatAttributeSet* GetAMBCombatAttributeSet() const { return CombatAttributeSet; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAMBAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAMBCombatAttributeSet> CombatAttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Damage")
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void PrepareAttackTrace();
	virtual bool TryGetAttackHitResult(FName TraceStartBone, FName TraceEndBone, FHitResult& OutHitResult) const;
	virtual bool ApplyAttackHitResult(const FHitResult& HitResult);

	void InitializeAbilityActorInfo();

	virtual void HandleHealthChanged(float OldHealth, float NewHealth, AActor* InstigatorActor);
	virtual void HandleOutOfHealth(AActor* InstigatorActor);

private:
	bool ApplyHealthDeltaToTarget(
		UAbilitySystemComponent* TargetAbilitySystem,
		float HealthDelta,
		AActor* InstigatorActor,
		UObject* SourceObject);

	FDelegateHandle HealthChangedDelegateHandle;
	bool bIsDead = false;
	TWeakObjectPtr<AActor> LastDamageCauser;
	FVector LastDamageLocation = FVector::ZeroVector;
	FVector LastDamageImpulse = FVector::ZeroVector;
};

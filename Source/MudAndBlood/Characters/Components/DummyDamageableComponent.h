#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DummyDamageableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FDummyDamageReceivedSignature, float, Damage, float, CurrentHealth, AActor*, DamageCauser, FVector, DamageLocation, FVector, DamageImpulse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDummyHealingReceivedSignature, float, Healing, float, CurrentHealth, AActor*, Healer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDummyDeathSignature, AActor*, DamageCauser);

/**
 * Reusable health component for dummy targets that should react to melee hit traces.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class MUDANDBLOOD_API UDummyDamageableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDummyDamageableComponent();

	/** Apply damage to this component's owner. Returns the actual damage applied after clamping. */
	UFUNCTION(BlueprintCallable, Category="Dummy|Damage")
	float ApplyDamage(float Damage, AActor* DamageCauser, FVector DamageLocation, FVector DamageImpulse);

	/** Heal this component's owner. Returns the actual healing applied after clamping. */
	UFUNCTION(BlueprintCallable, Category="Dummy|Damage")
	float ApplyHealing(float Healing, AActor* Healer);

	/** Restores the owner to full health and clears the dead state. */
	UFUNCTION(BlueprintCallable, Category="Dummy|Damage")
	void ResetHealth();

	/** Marks the owner as dead and performs the configured death behavior. */
	UFUNCTION(BlueprintCallable, Category="Dummy|Damage")
	void HandleDeath(AActor* DamageCauser);

	UFUNCTION(BlueprintPure, Category="Dummy|Damage")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category="Dummy|Damage")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category="Dummy|Damage")
	bool IsDead() const { return bIsDead; }

	UPROPERTY(BlueprintAssignable, Category="Dummy|Damage")
	FDummyDamageReceivedSignature OnDamaged;

	UPROPERTY(BlueprintAssignable, Category="Dummy|Damage")
	FDummyHealingReceivedSignature OnHealed;

	UPROPERTY(BlueprintAssignable, Category="Dummy|Damage")
	FDummyDeathSignature OnDeath;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dummy|Damage", meta=(ClampMin="0.0"))
	float MaxHealth = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Dummy|Damage", meta=(ClampMin="0.0"))
	float CurrentHealth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dummy|Damage")
	bool bResetHealthOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dummy|Damage")
	bool bDisableCollisionOnDeath = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dummy|Damage")
	bool bDestroyOwnerOnDeath = false;

private:
	bool bIsDead = false;
};

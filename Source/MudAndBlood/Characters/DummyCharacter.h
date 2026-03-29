// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatDamageable.h"
#include "DummyCharacter.generated.h"

class UDummyDamageableComponent;

UCLASS()
class MUDANDBLOOD_API ADummyCharacter : public ACharacter, public ICombatDamageable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADummyCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UDummyDamageableComponent> DummyDamageableComponent;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleDummyDamaged(float Damage, float CurrentHealth, AActor* DamageCauser, FVector DamageLocation, FVector DamageImpulse);

	UFUNCTION()
	void HandleDummyDied(AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent, Category="Dummy|Damage", meta=(DisplayName="On Dummy Damaged"))
	void BP_OnDummyDamaged(float Damage, float CurrentHealth, FVector DamageLocation, FVector DamageDirection, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent, Category="Dummy|Damage", meta=(DisplayName="On Dummy Died"))
	void BP_OnDummyDied(AActor* DamageCauser);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;
	virtual void HandleDeath() override;
	virtual void ApplyHealing(float Healing, AActor* Healer) override;
	virtual void NotifyDanger(const FVector& DangerLocation, AActor* DangerSource) override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category="Dummy|Damage")
	UDummyDamageableComponent* GetDummyDamageableComponent() const { return DummyDamageableComponent; }
};

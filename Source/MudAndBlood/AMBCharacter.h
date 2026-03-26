// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatAttacker.h"
#include "AMBCharacter.generated.h"

class UCombatAttackComponent;
class UInputAction;

UCLASS()
class MUDANDBLOOD_API AAMBCharacter : public ACharacter, public ICombatAttacker
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAMBCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatAttackComponent> CombatAttackComponent;

	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> ComboAttackAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> ChargedAttackAction;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ComboAttackPressed();
	void ChargedAttackPressed();
	void ChargedAttackReleased();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoComboAttackStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoComboAttackEnd();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoChargedAttackStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoChargedAttackEnd();

	virtual void DoAttackTrace(FName DamageSourceBone) override;
	virtual void CheckCombo() override;
	virtual void CheckChargedAttack() override;

	FORCEINLINE UCombatAttackComponent* GetCombatAttackComponent() const { return CombatAttackComponent; }

};

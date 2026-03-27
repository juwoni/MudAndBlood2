// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "CombatAttacker.h"
#include "AMBCharacter.generated.h"

class UAbilitySystemComponent;
class UAMBAbilitySystemComponent;
class UAMBCombatAttributeSet;
class UAMBCombatStyleData;
class UCombatAttackComponent;
class UInputAction;

UENUM(BlueprintType)
enum class EAMBCombatStyleType : uint8
{
	Unarmed,
	Sword,
	Bow
};

UCLASS()
class MUDANDBLOOD_API AAMBCharacter : public ACharacter, public IAbilitySystemInterface, public ICombatAttacker
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAMBCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAMBAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAMBCombatAttributeSet> CombatAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatAttackComponent> CombatAttackComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Style")
	TObjectPtr<UAMBCombatStyleData> DefaultCombatStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Style")
	TObjectPtr<UAMBCombatStyleData> UnarmedCombatStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Style")
	TObjectPtr<UAMBCombatStyleData> SwordCombatStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Style")
	TObjectPtr<UAMBCombatStyleData> BowCombatStyle;

	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> ComboAttackAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> ChargedAttackAction;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Transient)
	TObjectPtr<UAMBCombatStyleData> CurrentCombatStyle;

	FGameplayTag CurrentCombatStyleTag;

	TArray<FGameplayAbilitySpecHandle> GrantedCombatAbilityHandles;

	void ComboAttackPressed();
	void ChargedAttackPressed();
	void ChargedAttackReleased();

	void InitializeAbilityActorInfo();
	void ClearGrantedCombatAbilities();
	void GrantCombatStyleAbilities(const UAMBCombatStyleData* CombatStyleData);
	void UpdateCombatStyleTag(const FGameplayTag& NewCombatStyleTag);
	bool TryActivateCombatAbilityByInputTag(const FGameplayTag& InputTag) const;
	UAMBCombatStyleData* GetConfiguredCombatStyle(EAMBCombatStyleType CombatStyleType) const;

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

	UFUNCTION(BlueprintCallable, Category="Combat|Style")
	void SetCombatStyle(UAMBCombatStyleData* NewCombatStyle);

	UFUNCTION(BlueprintCallable, Category="Combat|Style")
	void EquipCombatStyleByType(EAMBCombatStyleType CombatStyleType);

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	FGameplayTag GetCurrentCombatStyleTag() const { return CurrentCombatStyleTag; }

	virtual void DoAttackTrace(FName DamageSourceBone) override;
	virtual void CheckCombo() override;
	virtual void CheckChargedAttack() override;

	FORCEINLINE UAMBAbilitySystemComponent* GetAMBAbilitySystemComponent() const { return AbilitySystemComponent; }
	FORCEINLINE UCombatAttackComponent* GetCombatAttackComponent() const { return CombatAttackComponent; }

};

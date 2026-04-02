// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "Characters/AMBGASCharacterBase.h"
#include "Variant_Combat/Data/AMBCombatStyleData.h"
#include "AMBCharacter.generated.h"

class UAMBInventoryComponent;
class UCombatAttackComponent;
class UInputAction;
class UAMBItemData;
class USceneComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAMBCombatStyleChangedSignature, int32, SlotIndex, EAMBCombatStyleType, CombatStyleType, UAMBCombatStyleData*, CombatStyleData);

UCLASS()
class MUDANDBLOOD_API AAMBCharacter : public AAMBGASCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAMBCharacter();

	UPROPERTY(BlueprintAssignable, Category="Combat|Style")
	FAMBCombatStyleChangedSignature OnCombatStyleChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAMBInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> EquippedItemMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Style")
	TObjectPtr<UAMBCombatStyleData> DefaultCombatStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Style")
	TObjectPtr<UAMBCombatStyleData> UnarmedCombatStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Style")
	TObjectPtr<UAMBCombatStyleData> SwordCombatStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Style")
	TObjectPtr<UAMBCombatStyleData> BowCombatStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Slots")
	TObjectPtr<UAMBCombatStyleData> CombatSlot1Style;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Slots")
	TObjectPtr<UAMBCombatStyleData> CombatSlot2Style;

	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> ComboAttackAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> ChargedAttackAction;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Transient)
	TObjectPtr<UAMBCombatStyleData> CurrentCombatStyle;

	FGameplayTag CurrentCombatStyleTag;
	EAMBCombatStyleType CurrentCombatStyleType = EAMBCombatStyleType::Unarmed;
	int32 CurrentCombatSlotIndex = INDEX_NONE;

	TArray<FGameplayAbilitySpecHandle> GrantedCombatAbilityHandles;

	void ComboAttackPressed();
	void ChargedAttackPressed();
	void ChargedAttackReleased();

	void ClearGrantedCombatAbilities();
	void GrantCombatStyleAbilities(const UAMBCombatStyleData* CombatStyleData);
	USceneComponent* GetEquippedItemAttachComponent() const;
	void UpdateEquippedItemMesh(UAMBItemData* ItemData);
	void UpdateCombatStyleTag(const FGameplayTag& NewCombatStyleTag);
	bool TryActivateCombatAbilityByInputTag(const FGameplayTag& InputTag) const;
	UAMBCombatStyleData* GetConfiguredCombatStyle(EAMBCombatStyleType CombatStyleType) const;
	UAMBCombatStyleData* GetConfiguredCombatSlotStyle(int32 SlotIndex) const;
	EAMBCombatStyleType ResolveCombatStyleType(const UAMBCombatStyleData* CombatStyleData) const;

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
	void ProcessAttack();
	

	UFUNCTION(BlueprintCallable, Category="Combat|Style")
	void SetCombatStyle(UAMBCombatStyleData* NewCombatStyle);

	UFUNCTION(BlueprintCallable, Category="Combat|Style")
	void SetDefaultCombatStyle(UAMBCombatStyleData* NewDefaultCombatStyle, int32 SourceSlotIndex);

	UFUNCTION(BlueprintCallable, Category="Combat|Style")
	void EquipCombatStyleByType(EAMBCombatStyleType CombatStyleType);

	UFUNCTION(BlueprintCallable, Category="Combat|Style")
	void SetAttackType(EAMBCombatStyleType CombatStyleType);

	UFUNCTION(BlueprintCallable, Category="Combat|Style")
	void EquipCombatSlot(int32 SlotIndex);

	UFUNCTION()
	void HandleInventorySlotSelected(int32 SlotIndex, UAMBItemData* ItemData);

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	FGameplayTag GetCurrentCombatStyleTag() const { return CurrentCombatStyleTag; }

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	EAMBCombatStyleType GetCurrentCombatStyleType() const { return CurrentCombatStyleType; }

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	int32 GetCurrentCombatSlotIndex() const { return CurrentCombatSlotIndex; }

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	UAMBCombatStyleData* GetCurrentCombatStyle() const { return CurrentCombatStyle; }

	UFUNCTION(BlueprintPure, Category="Inventory")
	UStaticMeshComponent* GetEquippedWeaponMesh() const { return EquippedItemMeshComponent; }

	UFUNCTION(BlueprintPure, Category="Combat|Damage")
	UAMBItemData* GetSelectedItemData() const;

	UFUNCTION(BlueprintPure, Category="Combat|Damage")
	float GetCurrentEquippedItemBaseDamage() const;

	UFUNCTION(BlueprintPure, Category="Combat|Damage")
	float GetCurrentWeaponDamage() const;

	UFUNCTION(BlueprintCallable, Category="Combat|Damage")
	bool ApplyCurrentWeaponDamageToTarget(AActor* TargetActor, FVector DamageLocation, FVector DamageImpulse);

	virtual bool SphereTraceMultiForObjects(FName TraceStartBone, FName TraceEndBone, AActor*& HitActor, FVector& ImpactPoint) override;
	virtual void DoAttackTrace(FName TraceStartBone, FName TraceEndBone) override;
	virtual void BeginAttackTraceWindow(FName TraceStartBone, FName TraceEndBone) override;
	virtual void TickAttackTraceWindow(FName TraceStartBone, FName TraceEndBone) override;
	virtual void EndAttackTraceWindow() override;
	virtual void CheckCombo() override;
	virtual void CheckChargedAttack() override;

	UCombatAttackComponent* GetCombatAttackComponent() const;
	FORCEINLINE UAMBInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	FORCEINLINE UStaticMeshComponent* GetEquippedItemMeshComponent() const { return EquippedItemMeshComponent; }

};

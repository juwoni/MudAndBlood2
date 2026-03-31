// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Characters/AMBGASCharacterBase.h"
#include "Variant_Combat/Data/AMBCombatStyleData.h"
#include "AMBCharacter.generated.h"

class UAMBCombatStyleComponent;
class UAMBEquipmentVisualComponent;
class UAMBInventoryComponent;
class UCombatAttackComponent;
class UInputAction;
class UAMBItemData;
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
	TObjectPtr<UCombatAttackComponent> CombatAttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAMBInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAMBEquipmentVisualComponent> EquippedItemMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAMBCombatStyleComponent> CombatStyleComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Style")
	TObjectPtr<UAMBCombatStyleData> UnarmedCombatStyle;

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
	void HandleCombatStyleChanged(int32 SlotIndex, EAMBCombatStyleType CombatStyleType,
	                              UAMBCombatStyleData* CombatStyleData);

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	FGameplayTag GetCurrentCombatStyleTag() const;

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	EAMBCombatStyleType GetCurrentCombatStyleType() const;

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	int32 GetCurrentCombatSlotIndex() const;

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	UAMBCombatStyleData* GetCurrentCombatStyle() const;

	UFUNCTION(BlueprintPure, Category="Inventory")
	UStaticMeshComponent* GetEquippedWeaponMesh() const;

	UFUNCTION(BlueprintPure, Category="Combat|Damage")
	UAMBItemData* GetSelectedItemData() const;

	UFUNCTION(BlueprintPure, Category="Combat|Damage")
	float GetCurrentEquippedItemBaseDamage() const;

	UFUNCTION(BlueprintPure, Category="Combat|Damage")
	float GetCurrentWeaponDamage() const;

	UFUNCTION(BlueprintCallable, Category="Combat|Damage")
	bool ApplyCurrentWeaponDamageToTarget(AActor* TargetActor, FVector DamageLocation, FVector DamageImpulse);

	virtual void DoAttackTrace(FName TraceStartBone, FName TraceEndBone) override;
	virtual void BeginAttackTraceWindow(FName TraceStartBone, FName TraceEndBone) override;
	virtual void TickAttackTraceWindow(FName TraceStartBone, FName TraceEndBone) override;
	virtual void EndAttackTraceWindow() override;
	virtual void CheckCombo() override;
	virtual void CheckChargedAttack() override;

	FORCEINLINE UCombatAttackComponent* GetCombatAttackComponent() const { return CombatAttackComponent; }
	FORCEINLINE UAMBInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	FORCEINLINE UAMBCombatStyleComponent* GetCombatStyleComponent() const { return CombatStyleComponent; }
	UStaticMeshComponent* GetEquippedItemMeshComponent() const;
	UAMBCombatStyleData* GetUnarmedConfiguredCombatStyle() const { return UnarmedCombatStyle; }

};

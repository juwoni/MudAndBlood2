#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AMBCharacter.h"
#include "AMBHUDWidget.generated.h"

class AAMBCharacter;
class UAMBInventoryComponent;
class UAMBCombatStyleData;
class UAMBItemData;

UCLASS(Abstract, Blueprintable)
class MUDANDBLOOD_API UAMBHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category="HUD|Combat")
	void EquipCombatSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category="HUD|Combat")
	void RefreshCombatStyle();

	UFUNCTION(BlueprintCallable, Category="HUD|Inventory")
	void SelectInventorySlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category="HUD|Inventory")
	void SetInventorySlotItem(int32 SlotIndex, UAMBItemData* ItemData);

	UFUNCTION(BlueprintPure, Category="HUD|Combat")
	AAMBCharacter* GetAMBCharacter() const;

	UFUNCTION(BlueprintPure, Category="HUD|Inventory")
	UAMBInventoryComponent* GetInventoryComponent() const;

	UFUNCTION(BlueprintPure, Category="HUD|Combat")
	int32 GetCurrentCombatSlotIndex() const;

	UFUNCTION(BlueprintPure, Category="HUD|Combat")
	EAMBCombatStyleType GetCurrentCombatStyleType() const;

	UFUNCTION(BlueprintPure, Category="HUD|Inventory")
	int32 GetSelectedInventorySlotIndex() const;

	UFUNCTION(BlueprintPure, Category="HUD|Inventory")
	UAMBItemData* GetInventorySlotItem(int32 SlotIndex) const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="HUD|Combat")
	void BP_OnCombatStyleChanged(int32 SlotIndex, EAMBCombatStyleType CombatStyleType, UAMBCombatStyleData* CombatStyleData);

private:
	UFUNCTION()
	void HandleCombatStyleChanged(int32 SlotIndex, EAMBCombatStyleType CombatStyleType, UAMBCombatStyleData* CombatStyleData);

	void BindToCharacter();
	void UnbindFromCharacter();

	UPROPERTY(Transient)
	TObjectPtr<AAMBCharacter> CachedCharacter;
};

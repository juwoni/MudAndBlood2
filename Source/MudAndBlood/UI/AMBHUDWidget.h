#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AMBCharacter.h"
#include "AMBHUDWidget.generated.h"

class UHorizontalBox;
class AAMBCharacter;
class UAMBInventoryComponent;
class UAMBCombatStyleData;
class UAMBItemData;
class UItemBoxWidget;
// class UHorizontalBox;

UCLASS(Abstract, Blueprintable)
class MUDANDBLOOD_API UAMBHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category="HUD|Inventory")
	TObjectPtr<UHorizontalBox> ItemBoxList;

	UPROPERTY(BlueprintReadWrite, Category="HUD|Inventory")
	TArray<TObjectPtr<UItemBoxWidget>> ItemBoxArray;

	UPROPERTY(BlueprintReadWrite, Category="HUD|Inventory")
	int32 SelectedItemBoxIndex;

	UFUNCTION(BlueprintCallable, Category="HUD|Inventory")
	void UpdateBoxes();

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

public:
	// UFUNCTION(BlueprintImplementableEvent)
	// void SetItemBoxName(const FString& NewItemBoxName, int32 NewItemBoxIndex);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="HUD|Combat")
	void BP_OnCombatStyleChanged(int32 SlotIndex, EAMBCombatStyleType CombatStyleType,
	                             UAMBCombatStyleData* CombatStyleData);

private:
	UFUNCTION()
	void HandleCombatStyleChanged(int32 SlotIndex, EAMBCombatStyleType CombatStyleType,
	                              UAMBCombatStyleData* CombatStyleData);

	UFUNCTION()
	void HandleInventorySlotSelected(int32 SlotIndex, UAMBItemData* ItemData);

	void BindToCharacter();
	void UnbindFromCharacter();

	UPROPERTY(Transient)
	TObjectPtr<AAMBCharacter> CachedCharacter;
};

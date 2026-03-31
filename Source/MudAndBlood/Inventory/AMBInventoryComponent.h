#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AMBInventoryComponent.generated.h"

class UAMBItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAMBInventorySlotChangedSignature, int32, SlotIndex, UAMBItemData*, ItemData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAMBInventorySlotSelectedSignature, int32, SlotIndex, UAMBItemData*, ItemData);

UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class MUDANDBLOOD_API UAMBInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAMBInventoryComponent();

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FAMBInventorySlotChangedSignature OnInventorySlotChanged;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FAMBInventorySlotSelectedSignature OnInventorySlotSelected;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void InitializeInventory(int32 NewSlotCount);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool SetItemInSlot(int32 SlotIndex, UAMBItemData* ItemData);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool SelectInventorySlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetInventorySlotCount() const { return InventoryItems.Num(); }

	UFUNCTION(BlueprintPure, Category="Inventory")
	UAMBItemData* GetItemInSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetSelectedSlotIndex() const { return SelectedSlotIndex; }

	UFUNCTION(BlueprintPure, Category="Inventory")
	UAMBItemData* GetSelectedItem() const;

protected:
	virtual void BeginPlay() override;
	void AddItem(UAMBItemData* NewItem);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory", meta=(ClampMin=1))
	int32 DefaultSlotCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory", meta=(AllowPrivateAccess="true"))
	TArray<TObjectPtr<UAMBItemData>> InventoryItems;

private:
	bool AddItemToSlot(int32 SlotIndex, UAMBItemData* ItemData, bool bBroadcastInventoryChanged = true);
	bool IsValidInventorySlot(int32 SlotIndex) const;

	UPROPERTY(Transient)
	int32 SelectedSlotIndex = INDEX_NONE;
};

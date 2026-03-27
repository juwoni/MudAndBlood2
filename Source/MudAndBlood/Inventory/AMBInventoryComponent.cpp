#include "Inventory/AMBInventoryComponent.h"

#include "AMBCharacter.h"
#include "Inventory/AMBItemData.h"

UAMBInventoryComponent::UAMBInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAMBInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (InventoryItems.Num() < DefaultSlotCount)
	{
		InventoryItems.SetNum(DefaultSlotCount);
	}
}

void UAMBInventoryComponent::InitializeInventory(int32 NewSlotCount)
{
	if (NewSlotCount <= 0)
	{
		return;
	}

	InventoryItems.SetNum(NewSlotCount);

	if (!IsValidInventorySlot(SelectedSlotIndex))
	{
		SelectedSlotIndex = INDEX_NONE;
	}
}

bool UAMBInventoryComponent::SetItemInSlot(int32 SlotIndex, UAMBItemData* ItemData)
{
	if (!IsValidInventorySlot(SlotIndex))
	{
		return false;
	}

	InventoryItems[SlotIndex] = ItemData;
	OnInventorySlotChanged.Broadcast(SlotIndex, ItemData);

	if (SelectedSlotIndex == SlotIndex)
	{
		ApplyCombatStyleFromItem(SlotIndex, ItemData);
		OnInventorySlotSelected.Broadcast(SlotIndex, ItemData);
	}

	return true;
}

bool UAMBInventoryComponent::SelectInventorySlot(int32 SlotIndex)
{
	
	if (!IsValidInventorySlot(SlotIndex))
	{
		return false;
	}

	SelectedSlotIndex = SlotIndex;

	UAMBItemData* SelectedItem = GetItemInSlot(SlotIndex);
	ApplyCombatStyleFromItem(SlotIndex, SelectedItem);
	OnInventorySlotSelected.Broadcast(SlotIndex, SelectedItem);

	return true;
}

UAMBItemData* UAMBInventoryComponent::GetItemInSlot(int32 SlotIndex) const
{
	
	if (!IsValidInventorySlot(SlotIndex))
	{
		return nullptr;
	}

	return InventoryItems[SlotIndex];
}

UAMBItemData* UAMBInventoryComponent::GetSelectedItem() const
{
	return GetItemInSlot(SelectedSlotIndex);
}

bool UAMBInventoryComponent::IsValidInventorySlot(int32 SlotIndex) const
{
	return InventoryItems.IsValidIndex(SlotIndex);
}

void UAMBInventoryComponent::ApplyCombatStyleFromItem(int32 SlotIndex, UAMBItemData* ItemData)
{
	if (!ItemData || !ItemData->CanApplyCombatStyleOnSelect())
	{
		return;
	}

	if (AAMBCharacter* Character = GetOwnerCharacter())
	{
		Character->SetDefaultCombatStyle(ItemData->CombatStyleData, SlotIndex);
	}
}

AAMBCharacter* UAMBInventoryComponent::GetOwnerCharacter() const
{
	return Cast<AAMBCharacter>(GetOwner());
}

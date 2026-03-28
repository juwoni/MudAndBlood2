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

void UAMBInventoryComponent::AddItem(UAMBItemData* NewItem)
{
	if (!NewItem)
	{
		return;
	}

	const int32 EmptySlotIndex = InventoryItems.IndexOfByPredicate([](const TObjectPtr<UAMBItemData>& Item)
	{
		return Item == nullptr;
	});

	if (EmptySlotIndex != INDEX_NONE)
	{
		AddItemToSlot(EmptySlotIndex, NewItem);
		return;
	}

	const int32 NewSlotIndex = InventoryItems.Add(NewItem);
	OnInventorySlotChanged.Broadcast(NewSlotIndex, NewItem);
	OnInventoryChanged.Broadcast();
}


void UAMBInventoryComponent::InitializeInventory(int32 NewSlotCount)
{
	static_cast<void>(NewSlotCount);
	InventoryItems.Init(nullptr, DefaultSlotCount);

	if (!IsValidInventorySlot(SelectedSlotIndex))
	{
		SelectedSlotIndex = INDEX_NONE;
	}

	UAMBItemData* Item1 = LoadObject<UAMBItemData>(
		nullptr,
		TEXT("/Game/MudAndBlood/DA_Item_Sword.DA_Item_Sword")
	);

	UAMBItemData* Item2 = LoadObject<UAMBItemData>(
		nullptr,
		TEXT("/Game/MudAndBlood/DA_Item_Unarmed.DA_Item_Unarmed")
	);

	if (Item2)
	{
		AddItemToSlot(0, Item2, false);
	}

	if (Item1)
	{
		AddItemToSlot(1, Item1, false);
	}
	
	if (Item1)
	{
		AddItemToSlot(2, Item1, false);
	}

	OnInventoryChanged.Broadcast();
}

bool UAMBInventoryComponent::SetItemInSlot(int32 SlotIndex, UAMBItemData* ItemData)
{
	return AddItemToSlot(SlotIndex, ItemData);
}

bool UAMBInventoryComponent::AddItemToSlot(int32 SlotIndex, UAMBItemData* ItemData, bool bBroadcastInventoryChanged)
{
	if (!IsValidInventorySlot(SlotIndex))
	{
		return false;
	}

	InventoryItems[SlotIndex] = ItemData;
	OnInventorySlotChanged.Broadcast(SlotIndex, ItemData);

	if (bBroadcastInventoryChanged)
	{
		OnInventoryChanged.Broadcast();
	}

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

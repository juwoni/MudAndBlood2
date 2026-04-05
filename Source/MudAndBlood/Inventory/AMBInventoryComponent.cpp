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

	InitializeInventory(DefaultSlotCount);
}

bool UAMBInventoryComponent::AddItem(UAMBItemData* NewItem, bool bBroadcastInventoryChanged)
{
	if (!NewItem)
	{
		return false;
	}

	const int32 EmptySlotIndex = InventoryItems.IndexOfByPredicate([](const TObjectPtr<UAMBItemData>& Item)
	{
		return Item == nullptr;
	});

	if (EmptySlotIndex != INDEX_NONE)
	{
		return AddItemToSlot(EmptySlotIndex, NewItem, bBroadcastInventoryChanged);
	}

	const int32 NewSlotIndex = InventoryItems.Add(NewItem);
	OnInventorySlotChanged.Broadcast(NewSlotIndex, NewItem);
	if (bBroadcastInventoryChanged)
	{
		OnInventoryChanged.Broadcast();
	}

	return true;
}

bool UAMBInventoryComponent::AddItem(const TCHAR* ItemPath, bool bBroadcastInventoryChanged)
{
	if (!ItemPath)
	{
		return false;
	}

	UAMBItemData* ItemData = LoadObject<UAMBItemData>(nullptr, ItemPath);
	if (!ItemData)
	{
		return false;
	}

	return AddItem(ItemData, bBroadcastInventoryChanged);
}

bool UAMBInventoryComponent::AddItem(int32 SlotIndex, const TCHAR* ItemPath, bool bBroadcastInventoryChanged)
{
	if (!ItemPath)
	{
		return false;
	}

	UAMBItemData* ItemData = LoadObject<UAMBItemData>(nullptr, ItemPath);
	if (!ItemData)
	{
		return false;
	}

	return AddItemToSlot(SlotIndex, ItemData, bBroadcastInventoryChanged);
}

void UAMBInventoryComponent::InitializeInventory(int32 NewSlotCount)
{
	const TArray<TObjectPtr<UAMBItemData>> ConfiguredItems = InventoryItems;
	const int32 SlotCount = FMath::Max3(1, NewSlotCount, ConfiguredItems.Num());
	InventoryItems.Init(nullptr, SlotCount);

	SelectedSlotIndex = INDEX_NONE;

	for (int32 SlotIndex = 0; SlotIndex < ConfiguredItems.Num(); ++SlotIndex)
	{
		if (UAMBItemData* ItemData = ConfiguredItems[SlotIndex])
		{
			AddItemToSlot(SlotIndex, ItemData, false);
		}
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
		if (ItemData)
		{
			ApplyCombatStyleFromItem(SlotIndex, ItemData);
			OnInventorySlotSelected.Broadcast(SlotIndex, ItemData);
		}
		else
		{
			ClearSelectedSlot();
		}
	}

	return true;
}

bool UAMBInventoryComponent::SelectInventorySlot(int32 SlotIndex)
{
	if (!IsValidInventorySlot(SlotIndex))
	{
		return false;
	}

	if (SelectedSlotIndex == SlotIndex)
	{
		ClearSelectedSlot();
		return true;
	}

	SelectedSlotIndex = SlotIndex;

	UAMBItemData* SelectedItem = GetItemInSlot(SlotIndex);
	if (!SelectedItem)
	{
		ClearSelectedSlot();
		return true;
	}

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

void UAMBInventoryComponent::ClearSelectedSlot()
{
	SelectedSlotIndex = INDEX_NONE;

	if (AAMBCharacter* Character = GetOwnerCharacter())
	{
		Character->SetDefaultCombatStyle(nullptr, INDEX_NONE);
	}

	OnInventorySlotSelected.Broadcast(INDEX_NONE, nullptr);
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

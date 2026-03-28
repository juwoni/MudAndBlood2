#include "UI/AMBHUDWidget.h"

#include "AMBCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/AMBInventoryComponent.h"
#include "Inventory/AMBItemData.h"

#include "MudAndBlood.h"
#include "UAMBItemBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Widget.h"

void UAMBHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ItemBoxArray.Reset();

	if (!ItemBoxList)
	{
		return;
	}

	for (int32 i = 0; i < ItemBoxList->GetChildrenCount(); i++)
	{
		UWidget* Child = ItemBoxList->GetChildAt(i);

		if (!IsValid(Child))
		{
			continue;
		}

		UItemBoxWidget* Entry = Cast<UItemBoxWidget>(Child);
		if (!Entry)
		{
			continue;
		}

		// UMyItemEntry* Entry = CreateWidget<UHorizontalBox>(GetWorld(), EntryClass);
		// ItemBoxList->AddChildToHorizontalBox(Entry);

		ItemBoxArray.Add(Entry);

		FString Name = Entry->GetName();

		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Yellow,
			FString::Printf(TEXT("Hello World %s"), *Name)
		);
	}

	BindToCharacter();
	RefreshCombatStyle();
}

void UAMBHUDWidget::NativeDestruct()
{
	UnbindFromCharacter();

	Super::NativeDestruct();
}

void UAMBHUDWidget::UpdateBoxes()
{
	UAMBInventoryComponent* InventoryComponent = GetInventoryComponent();

	auto ItemBoxSize = ItemBoxArray.Num();

	for (int32 BoxIndex = 0; BoxIndex < ItemBoxSize; ++BoxIndex)
	{
		UItemBoxWidget* ItemBox = ItemBoxArray[BoxIndex];
		if (!ItemBox)
		{
			continue;
		}

		UAMBItemData* ItemData = InventoryComponent ? InventoryComponent->GetItemInSlot(BoxIndex) : nullptr;
		ItemBox->SetItemData(ItemData);
	}
}

void UAMBHUDWidget::EquipCombatSlot(int32 SlotIndex)
{
	if (AAMBCharacter* Character = GetAMBCharacter())
	{
		Character->EquipCombatSlot(SlotIndex);
		return;
	}

	UE_LOG(LogMudAndBlood, Warning, TEXT("%s cannot equip combat slot %d: owning AMBCharacter was not found."),
	       *GetNameSafe(this),
	       SlotIndex);
}

void UAMBHUDWidget::RefreshCombatStyle()
{
	if (AAMBCharacter* Character = GetAMBCharacter())
	{
		BP_OnCombatStyleChanged(Character->GetCurrentCombatSlotIndex(), Character->GetCurrentCombatStyleType(),
		                        Character->GetCurrentCombatStyle());
	}
}

void UAMBHUDWidget::SelectInventorySlot(int32 SlotIndex)
{
	if (UAMBInventoryComponent* InventoryComponent = GetInventoryComponent())
	{
		InventoryComponent->SelectInventorySlot(SlotIndex);
		return;
	}

	UE_LOG(LogMudAndBlood, Warning,
	       TEXT("%s cannot select inventory slot %d: owning inventory component was not found."),
	       *GetNameSafe(this),
	       SlotIndex);
}

void UAMBHUDWidget::SetInventorySlotItem(int32 SlotIndex, UAMBItemData* ItemData)
{
	if (UAMBInventoryComponent* InventoryComponent = GetInventoryComponent())
	{
		InventoryComponent->SetItemInSlot(SlotIndex, ItemData);
		return;
	}

	UE_LOG(LogMudAndBlood, Warning, TEXT("%s cannot set inventory slot %d: owning inventory component was not found."),
	       *GetNameSafe(this),
	       SlotIndex);
}

AAMBCharacter* UAMBHUDWidget::GetAMBCharacter() const
{
	if (CachedCharacter)
	{
		return CachedCharacter;
	}

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		return Cast<AAMBCharacter>(PlayerController->GetPawn());
	}

	return nullptr;
}

UAMBInventoryComponent* UAMBHUDWidget::GetInventoryComponent() const
{
	if (const AAMBCharacter* Character = GetAMBCharacter())
	{
		return Character->GetInventoryComponent();
	}

	return nullptr;
}

int32 UAMBHUDWidget::GetCurrentCombatSlotIndex() const
{
	if (const AAMBCharacter* Character = GetAMBCharacter())
	{
		return Character->GetCurrentCombatSlotIndex();
	}

	return INDEX_NONE;
}

EAMBCombatStyleType UAMBHUDWidget::GetCurrentCombatStyleType() const
{
	if (const AAMBCharacter* Character = GetAMBCharacter())
	{
		return Character->GetCurrentCombatStyleType();
	}

	return EAMBCombatStyleType::Unarmed;
}

int32 UAMBHUDWidget::GetSelectedInventorySlotIndex() const
{
	if (const UAMBInventoryComponent* InventoryComponent = GetInventoryComponent())
	{
		return InventoryComponent->GetSelectedSlotIndex();
	}

	return INDEX_NONE;
}

UAMBItemData* UAMBHUDWidget::GetInventorySlotItem(int32 SlotIndex) const
{
	if (const UAMBInventoryComponent* InventoryComponent = GetInventoryComponent())
	{
		return InventoryComponent->GetItemInSlot(SlotIndex);
	}

	return nullptr;
}

void UAMBHUDWidget::HandleCombatStyleChanged(int32 SlotIndex, EAMBCombatStyleType CombatStyleType,
                                             UAMBCombatStyleData* CombatStyleData)
{
	BP_OnCombatStyleChanged(SlotIndex, CombatStyleType, CombatStyleData);
}

void UAMBHUDWidget::HandleInventorySlotSelected(int32 SlotIndex, UAMBItemData* ItemData)
{
	static_cast<void>(ItemData);
	SelectedItemBoxIndex = SlotIndex;
}

void UAMBHUDWidget::BindToCharacter()
{
	UnbindFromCharacter();

	CachedCharacter = GetAMBCharacter();
	if (!IsValid(CachedCharacter))
	{
		return;
	}

	CachedCharacter->OnCombatStyleChanged.AddDynamic(this, &UAMBHUDWidget::HandleCombatStyleChanged);

	if (UAMBInventoryComponent* InventoryComponent = CachedCharacter->GetInventoryComponent())
	{
		InventoryComponent->OnInventoryChanged.AddDynamic(this, &UAMBHUDWidget::UpdateBoxes);
		InventoryComponent->OnInventorySlotSelected.AddDynamic(this, &UAMBHUDWidget::HandleInventorySlotSelected);
		UpdateBoxes();
		SelectedItemBoxIndex = InventoryComponent->GetSelectedSlotIndex();
	}
}

void UAMBHUDWidget::UnbindFromCharacter()
{
	if (!IsValid(CachedCharacter))
	{
		return;
	}

	if (UAMBInventoryComponent* InventoryComponent = CachedCharacter->GetInventoryComponent())
	{
		InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UAMBHUDWidget::UpdateBoxes);
		InventoryComponent->OnInventorySlotSelected.RemoveDynamic(this, &UAMBHUDWidget::HandleInventorySlotSelected);
	}

	CachedCharacter->OnCombatStyleChanged.RemoveDynamic(this, &UAMBHUDWidget::HandleCombatStyleChanged);
	CachedCharacter = nullptr;
}

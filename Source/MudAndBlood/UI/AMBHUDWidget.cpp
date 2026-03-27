#include "UI/AMBHUDWidget.h"

#include "AMBCharacter.h"
#include "GameFramework/PlayerController.h"
#include "MudAndBlood.h"

void UAMBHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToCharacter();
	RefreshCombatStyle();
}

void UAMBHUDWidget::NativeDestruct()
{
	UnbindFromCharacter();

	Super::NativeDestruct();
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
		BP_OnCombatStyleChanged(Character->GetCurrentCombatSlotIndex(), Character->GetCurrentCombatStyleType(), Character->GetCurrentCombatStyle());
	}
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

void UAMBHUDWidget::HandleCombatStyleChanged(int32 SlotIndex, EAMBCombatStyleType CombatStyleType, UAMBCombatStyleData* CombatStyleData)
{
	BP_OnCombatStyleChanged(SlotIndex, CombatStyleType, CombatStyleData);
}

void UAMBHUDWidget::BindToCharacter()
{
	UnbindFromCharacter();

	CachedCharacter = GetAMBCharacter();
	if (!CachedCharacter)
	{
		return;
	}

	CachedCharacter->OnCombatStyleChanged.AddDynamic(this, &UAMBHUDWidget::HandleCombatStyleChanged);
}

void UAMBHUDWidget::UnbindFromCharacter()
{
	if (!CachedCharacter)
	{
		return;
	}

	CachedCharacter->OnCombatStyleChanged.RemoveDynamic(this, &UAMBHUDWidget::HandleCombatStyleChanged);
	CachedCharacter = nullptr;
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "UAMBItemBox.h"

#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Inventory/AMBItemData.h"

void UItemBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetItemData(nullptr);
}

void UItemBoxWidget::SetItemData(UAMBItemData* ItemData)
{
	if (ItemNameText)
	{
		const FText DisplayName = ItemData
			                          ? (!ItemData->ItemName.IsEmpty()
				                             ? ItemData->ItemName
				                             : FText::FromString(ItemData->GetName()))
			                          : FText::GetEmpty();

		ItemNameText->SetText(DisplayName);
	}

	ItemContainer->SetVisibility(ItemData ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

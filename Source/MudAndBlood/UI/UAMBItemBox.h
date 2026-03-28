// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UAMBItemBox.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MUDANDBLOOD_API UItemBoxWidget : public UUserWidget
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;
	
public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category="HUD|Inventory")
	TObjectPtr<UTextBlock> ItemNameText;
};

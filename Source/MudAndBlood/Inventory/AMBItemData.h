#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Variant_Combat/Data/AMBCombatStyleData.h"
#include "AMBItemData.generated.h"

UCLASS(BlueprintType)
class MUDANDBLOOD_API UAMBItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Combat")
	TObjectPtr<UAMBCombatStyleData> CombatStyleData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Combat")
	bool bApplyCombatStyleOnSelect = true;

	UFUNCTION(BlueprintPure, Category="Item|Combat")
	bool CanApplyCombatStyleOnSelect() const
	{
		return bApplyCombatStyleOnSelect && CombatStyleData != nullptr;
	}
};

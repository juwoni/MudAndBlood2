#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Variant_Combat/Data/AMBCombatStyleData.h"
#include "AMBItemData.generated.h"

class UStaticMesh;

UCLASS(BlueprintType)
class MUDANDBLOOD_API UAMBItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Equip")
	TObjectPtr<UStaticMesh> EquippedMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Equip")
	FName EquippedMeshSocketName = TEXT("hand_rSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Equip")
	FVector EquippedMeshRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Equip")
	FRotator EquippedMeshRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Equip")
	FVector EquippedMeshRelativeScale = FVector(1.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Combat")
	TObjectPtr<UAMBCombatStyleData> CombatStyleData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Combat")
	EAMBCombatStyleType CombatStyleType = EAMBCombatStyleType::Unarmed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Combat", meta = (ClampMin = 0))
	float BaseDamage = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Combat")
	bool bApplyCombatStyleOnSelect = true;

	UFUNCTION(BlueprintPure, Category="Item|Combat")
	bool CanApplyCombatStyleOnSelect() const
	{
		return bApplyCombatStyleOnSelect && CombatStyleData != nullptr;
	}
};

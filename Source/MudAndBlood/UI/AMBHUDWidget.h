#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AMBCharacter.h"
#include "AMBHUDWidget.generated.h"

class AAMBCharacter;
class UAMBCombatStyleData;

UCLASS(Abstract, Blueprintable)
class MUDANDBLOOD_API UAMBHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category="HUD|Combat")
	void EquipCombatSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category="HUD|Combat")
	void RefreshCombatStyle();

	UFUNCTION(BlueprintPure, Category="HUD|Combat")
	AAMBCharacter* GetAMBCharacter() const;

	UFUNCTION(BlueprintPure, Category="HUD|Combat")
	int32 GetCurrentCombatSlotIndex() const;

	UFUNCTION(BlueprintPure, Category="HUD|Combat")
	EAMBCombatStyleType GetCurrentCombatStyleType() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="HUD|Combat")
	void BP_OnCombatStyleChanged(int32 SlotIndex, EAMBCombatStyleType CombatStyleType, UAMBCombatStyleData* CombatStyleData);

private:
	UFUNCTION()
	void HandleCombatStyleChanged(int32 SlotIndex, EAMBCombatStyleType CombatStyleType, UAMBCombatStyleData* CombatStyleData);

	void BindToCharacter();
	void UnbindFromCharacter();

	UPROPERTY(Transient)
	TObjectPtr<AAMBCharacter> CachedCharacter;
};

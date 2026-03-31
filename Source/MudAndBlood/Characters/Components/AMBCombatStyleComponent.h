#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Variant_Combat/Data/AMBCombatStyleData.h"
#include "AMBCombatStyleComponent.generated.h"

class UAMBCombatStyleData;
class UAMBInventoryComponent;
class UAMBItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAMBCombatStyleStateChangedSignature, int32, SlotIndex, EAMBCombatStyleType, CombatStyleType, UAMBCombatStyleData*, CombatStyleData);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class MUDANDBLOOD_API UAMBCombatStyleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAMBCombatStyleComponent();

	UPROPERTY(BlueprintAssignable, Category="Combat|Style")
	FAMBCombatStyleStateChangedSignature OnCombatStyleChanged;

	UFUNCTION(BlueprintCallable, Category="Combat|Style")
	void SetCombatStyle(UAMBCombatStyleData* NewCombatStyle);

	UFUNCTION(BlueprintCallable, Category="Combat|Style")
	void SetDefaultCombatStyle(UAMBCombatStyleData* NewDefaultCombatStyle, int32 SourceSlotIndex);

	UFUNCTION(BlueprintCallable, Category="Combat|Style")
	void EquipCombatStyleByType(EAMBCombatStyleType CombatStyleType);

	UFUNCTION(BlueprintCallable, Category="Combat|Style")
	void EquipCombatSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	FGameplayTag GetCurrentCombatStyleTag() const { return CurrentCombatStyleTag; }

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	EAMBCombatStyleType GetCurrentCombatStyleType() const { return CurrentCombatStyleType; }

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	int32 GetCurrentCombatSlotIndex() const { return CurrentCombatSlotIndex; }

	UFUNCTION(BlueprintPure, Category="Combat|Style")
	UAMBCombatStyleData* GetCurrentCombatStyle() const { return CurrentCombatStyle; }

	bool TryActivateCombatAbilityByInputTag(const FGameplayTag& InputTag) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void InitializeStartingCombatStyle();
	void ClearGrantedCombatAbilities();
	void GrantCombatStyleAbilities(const UAMBCombatStyleData* CombatStyleData);
	void UpdateCombatStyleTag(const FGameplayTag& NewCombatStyleTag);
	EAMBCombatStyleType ResolveCombatStyleType(const UAMBCombatStyleData* CombatStyleData) const;
	UAMBInventoryComponent* GetInventoryComponent() const;
	void LoadConfiguredStateFromOwner();

	UFUNCTION()
	void HandleInventorySlotSelected(int32 SlotIndex, UAMBItemData* ItemData);

	UPROPERTY(Transient)
	TObjectPtr<UAMBCombatStyleData> UnarmedCombatStyle;

	UPROPERTY(Transient)
	TObjectPtr<UAMBCombatStyleData> CurrentCombatStyle;

	FGameplayTag CurrentCombatStyleTag;
	EAMBCombatStyleType CurrentCombatStyleType = EAMBCombatStyleType::Unarmed;
	int32 CurrentCombatSlotIndex = INDEX_NONE;

	TArray<FGameplayAbilitySpecHandle> GrantedCombatAbilityHandles;
};

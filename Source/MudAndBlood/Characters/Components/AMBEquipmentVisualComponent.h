#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "AMBEquipmentVisualComponent.generated.h"

class UAMBItemData;
class UAMBInventoryComponent;
class USceneComponent;

UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class MUDANDBLOOD_API UAMBEquipmentVisualComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UAMBEquipmentVisualComponent();

	UFUNCTION(BlueprintCallable, Category="Inventory|Equip")
	void RefreshFromItem(UAMBItemData* ItemData);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	USceneComponent* ResolveAttachComponent() const;

private:
	UFUNCTION()
	void HandleInventorySlotSelected(int32 SlotIndex, UAMBItemData* ItemData);

	UAMBInventoryComponent* GetInventoryComponent() const;
};

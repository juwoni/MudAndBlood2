#include "Characters/Components/AMBEquipmentVisualComponent.h"

#include "AMBCharacter.h"
#include "Components/ChildActorComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "Inventory/AMBInventoryComponent.h"
#include "Inventory/AMBItemData.h"
#include "MudAndBlood.h"

UAMBEquipmentVisualComponent::UAMBEquipmentVisualComponent()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetHiddenInGame(true);
}

void UAMBEquipmentVisualComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UAMBInventoryComponent* InventoryComponent = GetInventoryComponent())
	{
		InventoryComponent->OnInventorySlotSelected.AddDynamic(this, &UAMBEquipmentVisualComponent::HandleInventorySlotSelected);
		RefreshFromItem(InventoryComponent->GetSelectedItem());
	}
}

void UAMBEquipmentVisualComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAMBInventoryComponent* InventoryComponent = GetInventoryComponent())
	{
		InventoryComponent->OnInventorySlotSelected.RemoveDynamic(this, &UAMBEquipmentVisualComponent::HandleInventorySlotSelected);
	}

	Super::EndPlay(EndPlayReason);
}

void UAMBEquipmentVisualComponent::RefreshFromItem(UAMBItemData* ItemData)
{
	UStaticMesh* EquippedMesh = ItemData ? ItemData->EquippedMesh.Get() : nullptr;
	if (!EquippedMesh)
	{
		UE_LOG(LogMudAndBlood, Log,
		       TEXT("%s cleared equipped item mesh. Item=%s"),
		       *GetNameSafe(GetOwner()),
		       *GetNameSafe(ItemData));

		SetStaticMesh(nullptr);
		SetHiddenInGame(true);
		return;
	}

	USceneComponent* AttachComponent = ResolveAttachComponent();
	if (!AttachComponent)
	{
		SetStaticMesh(nullptr);
		SetHiddenInGame(true);
		return;
	}

	SetStaticMesh(EquippedMesh);
	AttachToComponent(
		AttachComponent,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		ItemData->EquippedMeshSocketName
	);
	SetRelativeLocation(ItemData->EquippedMeshRelativeLocation);
	SetRelativeRotation(ItemData->EquippedMeshRelativeRotation);
	SetRelativeScale3D(ItemData->EquippedMeshRelativeScale);
	SetHiddenInGame(false);

	UE_LOG(LogMudAndBlood, Log,
	       TEXT("%s equipped item mesh updated. Item=%s Mesh=%s AttachSocket=%s"),
	       *GetNameSafe(GetOwner()),
	       *GetNameSafe(ItemData),
	       *GetNameSafe(EquippedMesh),
	       *ItemData->EquippedMeshSocketName.ToString());
}

USceneComponent* UAMBEquipmentVisualComponent::ResolveAttachComponent() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	TArray<UChildActorComponent*> ChildActorComponents;
	OwnerActor->GetComponents(ChildActorComponents);

	for (UChildActorComponent* ChildActorComponent : ChildActorComponents)
	{
		if (!IsValid(ChildActorComponent) || ChildActorComponent->GetName() != TEXT("VisualOverride"))
		{
			continue;
		}

		if (AActor* ChildActor = ChildActorComponent->GetChildActor())
		{
			if (UMeshComponent* MeshComponent = ChildActor->FindComponentByClass<UMeshComponent>())
			{
				return MeshComponent;
			}
		}

		break;
	}

	if (const ACharacter* CharacterOwner = Cast<ACharacter>(OwnerActor))
	{
		return CharacterOwner->GetMesh();
	}

	return OwnerActor->GetRootComponent();
}

void UAMBEquipmentVisualComponent::HandleInventorySlotSelected(int32 SlotIndex, UAMBItemData* ItemData)
{
	static_cast<void>(SlotIndex);
	RefreshFromItem(ItemData);
}

UAMBInventoryComponent* UAMBEquipmentVisualComponent::GetInventoryComponent() const
{
	if (const AAMBCharacter* CharacterOwner = Cast<AAMBCharacter>(GetOwner()))
	{
		return CharacterOwner->GetInventoryComponent();
	}

	return nullptr;
}

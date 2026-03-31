#include "Characters/Components/AMBCombatStyleComponent.h"

#include "AMBCharacter.h"
#include "AbilitySystem/AMBAbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Inventory/AMBInventoryComponent.h"
#include "Inventory/AMBItemData.h"
#include "MudAndBlood.h"
#include "Variant_Combat/Components/CombatAttackComponent.h"

UAMBCombatStyleComponent::UAMBCombatStyleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAMBCombatStyleComponent::InitializeStartingCombatStyle()
{
	if (UnarmedCombatStyle)
	{
		CurrentCombatSlotIndex = INDEX_NONE;
		SetCombatStyle(UnarmedCombatStyle);
	}
}

void UAMBCombatStyleComponent::SetCombatStyle(UAMBCombatStyleData* NewCombatStyle)
{
	if (CurrentCombatStyle == NewCombatStyle)
	{
		return;
	}

	ClearGrantedCombatAbilities();

	CurrentCombatStyle = NewCombatStyle;
	UpdateCombatStyleTag(CurrentCombatStyle ? CurrentCombatStyle->CombatStyleTag : FGameplayTag());
	CurrentCombatStyleType = ResolveCombatStyleType(CurrentCombatStyle);

	if (AAMBCharacter* Character = Cast<AAMBCharacter>(GetOwner()))
	{
		if (UCombatAttackComponent* CombatAttackComponent = Character->GetCombatAttackComponent())
		{
			if (CurrentCombatStyle)
			{
				CombatAttackComponent->ApplyCombatStyleData(CurrentCombatStyle);
			}
		}
	}

	GrantCombatStyleAbilities(CurrentCombatStyle);

	UE_LOG(LogMudAndBlood, Log, TEXT("%s switched combat style to %s (Tag=%s)."),
	       *GetNameSafe(GetOwner()),
	       *GetNameSafe(CurrentCombatStyle),
	       *CurrentCombatStyleTag.ToString());

	OnCombatStyleChanged.Broadcast(CurrentCombatSlotIndex, CurrentCombatStyleType, CurrentCombatStyle);
}

void UAMBCombatStyleComponent::SetDefaultCombatStyle(UAMBCombatStyleData* NewDefaultCombatStyle, int32 SourceSlotIndex)
{
	UAMBCombatStyleData* ResolvedCombatStyle = NewDefaultCombatStyle ? NewDefaultCombatStyle : UnarmedCombatStyle.Get();
	const bool bSlotChanged = CurrentCombatSlotIndex != SourceSlotIndex;
	const bool bStyleChanged = CurrentCombatStyle != ResolvedCombatStyle;

	CurrentCombatSlotIndex = SourceSlotIndex;

	SetCombatStyle(ResolvedCombatStyle);

	if (!bStyleChanged && bSlotChanged)
	{
		OnCombatStyleChanged.Broadcast(CurrentCombatSlotIndex, CurrentCombatStyleType, CurrentCombatStyle);
	}
}

void UAMBCombatStyleComponent::EquipCombatStyleByType(EAMBCombatStyleType CombatStyleType)
{
	if (CombatStyleType == EAMBCombatStyleType::Unarmed && UnarmedCombatStyle)
	{
		SetDefaultCombatStyle(UnarmedCombatStyle, INDEX_NONE);
		return;
	}

	static const UEnum* CombatStyleEnum = StaticEnum<EAMBCombatStyleType>();
	const FString CombatStyleName = CombatStyleEnum
		                                ? CombatStyleEnum->GetNameStringByValue(static_cast<int64>(CombatStyleType))
		                                : TEXT("Unknown");

	UE_LOG(LogMudAndBlood, Warning,
	       TEXT("%s cannot equip combat style %s directly: combat styles are driven by the selected inventory item."),
	       *GetNameSafe(GetOwner()),
	       *CombatStyleName);
}

void UAMBCombatStyleComponent::EquipCombatSlot(int32 SlotIndex)
{
	if (UAMBInventoryComponent* InventoryComponent = GetInventoryComponent())
	{
		InventoryComponent->SelectInventorySlot(SlotIndex);
	}
}

bool UAMBCombatStyleComponent::TryActivateCombatAbilityByInputTag(const FGameplayTag& InputTag) const
{
	const AAMBCharacter* Character = Cast<AAMBCharacter>(GetOwner());
	const UAMBAbilitySystemComponent* AbilitySystemComponent = Character ? Character->GetAMBAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogMudAndBlood, Warning,
		       TEXT("%s failed to activate combat input %s: AbilitySystemComponent is missing."),
		       *GetNameSafe(GetOwner()),
		       *InputTag.ToString());
		return false;
	}

	if (!AbilitySystemComponent->HasAbilityWithInputTag(InputTag))
	{
		UE_LOG(LogMudAndBlood, Warning,
		       TEXT("%s failed to activate combat input %s: no granted ability matches this input tag. CurrentStyle=%s"),
		       *GetNameSafe(GetOwner()),
		       *InputTag.ToString(),
		       *CurrentCombatStyleTag.ToString());
		return false;
	}

	const bool bActivated = const_cast<UAMBAbilitySystemComponent*>(AbilitySystemComponent)->TryActivateAbilitiesByInputTag(InputTag);
	if (!bActivated)
	{
		UE_LOG(LogMudAndBlood, Warning,
		       TEXT("%s failed to activate combat input %s: matching ability exists but activation was rejected. CurrentStyle=%s"),
		       *GetNameSafe(GetOwner()),
		       *InputTag.ToString(),
		       *CurrentCombatStyleTag.ToString());
	}

	return bActivated;
}

void UAMBCombatStyleComponent::BeginPlay()
{
	Super::BeginPlay();

	LoadConfiguredStateFromOwner();

	if (UAMBInventoryComponent* InventoryComponent = GetInventoryComponent())
	{
		InventoryComponent->OnInventorySlotSelected.AddDynamic(this, &UAMBCombatStyleComponent::HandleInventorySlotSelected);
	}

	InitializeStartingCombatStyle();
}

void UAMBCombatStyleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAMBInventoryComponent* InventoryComponent = GetInventoryComponent())
	{
		InventoryComponent->OnInventorySlotSelected.RemoveDynamic(this, &UAMBCombatStyleComponent::HandleInventorySlotSelected);
	}

	Super::EndPlay(EndPlayReason);
}

void UAMBCombatStyleComponent::ClearGrantedCombatAbilities()
{
	const AAMBCharacter* Character = Cast<AAMBCharacter>(GetOwner());
	UAMBAbilitySystemComponent* AbilitySystemComponent = Character ? Character->GetAMBAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent || !GetOwner() || !GetOwner()->HasAuthority())
	{
		GrantedCombatAbilityHandles.Reset();
		return;
	}

	for (const FGameplayAbilitySpecHandle& AbilityHandle : GrantedCombatAbilityHandles)
	{
		AbilitySystemComponent->ClearAbility(AbilityHandle);
	}

	GrantedCombatAbilityHandles.Reset();
}

void UAMBCombatStyleComponent::GrantCombatStyleAbilities(const UAMBCombatStyleData* CombatStyleData)
{
	const AAMBCharacter* Character = Cast<AAMBCharacter>(GetOwner());
	UAMBAbilitySystemComponent* AbilitySystemComponent = Character ? Character->GetAMBAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent || !CombatStyleData || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	for (const FAMBCombatAbilityGrant& AbilityGrant : CombatStyleData->GrantedAbilities)
	{
		if (!AbilityGrant.AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityGrant.AbilityClass, AbilityGrant.AbilityLevel);
		GrantedCombatAbilityHandles.Add(AbilitySystemComponent->GiveAbility(AbilitySpec));
	}
}

void UAMBCombatStyleComponent::UpdateCombatStyleTag(const FGameplayTag& NewCombatStyleTag)
{
	const AAMBCharacter* Character = Cast<AAMBCharacter>(GetOwner());
	UAMBAbilitySystemComponent* AbilitySystemComponent = Character ? Character->GetAMBAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent)
	{
		CurrentCombatStyleTag = NewCombatStyleTag;
		return;
	}

	if (CurrentCombatStyleTag.IsValid())
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CurrentCombatStyleTag);
	}

	CurrentCombatStyleTag = NewCombatStyleTag;

	if (CurrentCombatStyleTag.IsValid())
	{
		AbilitySystemComponent->AddLooseGameplayTag(CurrentCombatStyleTag);
	}
}

EAMBCombatStyleType UAMBCombatStyleComponent::ResolveCombatStyleType(const UAMBCombatStyleData* CombatStyleData) const
{
	if (!CombatStyleData)
	{
		return EAMBCombatStyleType::Unarmed;
	}

	return CombatStyleData->CombatStyleType;
}

UAMBInventoryComponent* UAMBCombatStyleComponent::GetInventoryComponent() const
{
	if (const AAMBCharacter* Character = Cast<AAMBCharacter>(GetOwner()))
	{
		return Character->GetInventoryComponent();
	}

	return nullptr;
}

void UAMBCombatStyleComponent::LoadConfiguredStateFromOwner()
{
	const AAMBCharacter* Character = Cast<AAMBCharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	UnarmedCombatStyle = Character->GetUnarmedConfiguredCombatStyle();
}

void UAMBCombatStyleComponent::HandleInventorySlotSelected(int32 SlotIndex, UAMBItemData* ItemData)
{
	if (ItemData && ItemData->CanApplyCombatStyleOnSelect())
	{
		SetDefaultCombatStyle(ItemData->CombatStyleData, SlotIndex);
		return;
	}

	SetDefaultCombatStyle(UnarmedCombatStyle, INDEX_NONE);
}

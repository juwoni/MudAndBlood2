// Fill out your copyright notice in the Description page of Project Settings.


#include "AMBCharacter.h"
#include "AbilitySystem/AMBAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AMBCombatAttributeSet.h"
#include "AbilitySystem/AMBGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/ChildActorComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "Inventory/AMBInventoryComponent.h"
#include "Inventory/AMBItemData.h"
#include "MudAndBlood.h"
#include "Variant_Combat/Components/CombatAttackComponent.h"

// Sets default values
AAMBCharacter::AAMBCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	CombatAttackComponent = CreateDefaultSubobject<UCombatAttackComponent>(TEXT("CombatAttackComponent"));
	InventoryComponent = CreateDefaultSubobject<UAMBInventoryComponent>(TEXT("InventoryComponent"));
	EquippedItemMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EquippedItemMeshComponent"));
	EquippedItemMeshComponent->SetupAttachment(GetMesh());
	EquippedItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquippedItemMeshComponent->SetGenerateOverlapEvents(false);
	EquippedItemMeshComponent->SetHiddenInGame(true);
}

// Called when the game starts or when spawned
void AAMBCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (InventoryComponent)
	{
		InventoryComponent->OnInventorySlotSelected.AddDynamic(this, &AAMBCharacter::HandleInventorySlotSelected);
		UpdateEquippedItemMesh(InventoryComponent->GetSelectedItem());
	}

	if (DefaultCombatStyle)
	{
		SetCombatStyle(DefaultCombatStyle);
		return;
	}

	if (CombatSlot1Style)
	{
		CurrentCombatSlotIndex = 1;
		SetCombatStyle(CombatSlot1Style);
		return;
	}

	if (UnarmedCombatStyle)
	{
		SetCombatStyle(UnarmedCombatStyle);
	}
}

// Called every frame
void AAMBCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AAMBCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this,
		                                   &AAMBCharacter::ComboAttackPressed);
		EnhancedInputComponent->BindAction(ChargedAttackAction, ETriggerEvent::Started, this,
		                                   &AAMBCharacter::ChargedAttackPressed);
		EnhancedInputComponent->BindAction(ChargedAttackAction, ETriggerEvent::Completed, this,
		                                   &AAMBCharacter::ChargedAttackReleased);
	}
}

void AAMBCharacter::ComboAttackPressed()
{
	DoComboAttackStart();
}

void AAMBCharacter::ChargedAttackPressed()
{
	DoChargedAttackStart();
}

void AAMBCharacter::ChargedAttackReleased()
{
	DoChargedAttackEnd();
}

void AAMBCharacter::ClearGrantedCombatAbilities()
{
	if (!AbilitySystemComponent || !HasAuthority())
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

void AAMBCharacter::GrantCombatStyleAbilities(const UAMBCombatStyleData* CombatStyleData)
{
	if (!AbilitySystemComponent || !CombatStyleData || !HasAuthority())
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

USceneComponent* AAMBCharacter::GetEquippedItemAttachComponent() const
{
	TArray<UChildActorComponent*> ChildActorComponents;
	GetComponents(ChildActorComponents);

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

	return GetMesh();
}

void AAMBCharacter::UpdateCombatStyleTag(const FGameplayTag& NewCombatStyleTag)
{
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

bool AAMBCharacter::TryActivateCombatAbilityByInputTag(const FGameplayTag& InputTag) const
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogMudAndBlood, Warning,
		       TEXT("%s failed to activate combat input %s: AbilitySystemComponent is missing."),
		       *GetNameSafe(this),
		       *InputTag.ToString());
		return false;
	}

	if (!AbilitySystemComponent->HasAbilityWithInputTag(InputTag))
	{
		UE_LOG(LogMudAndBlood, Warning,
		       TEXT("%s failed to activate combat input %s: no granted ability matches this input tag. CurrentStyle=%s"
		       ),
		       *GetNameSafe(this),
		       *InputTag.ToString(),
		       *CurrentCombatStyleTag.ToString());
		return false;
	}

	const bool bActivated = AbilitySystemComponent->TryActivateAbilitiesByInputTag(InputTag);
	if (!bActivated)
	{
		UE_LOG(LogMudAndBlood, Warning,
		       TEXT(
			       "%s failed to activate combat input %s: matching ability exists but activation was rejected. CurrentStyle=%s"
		       ),
		       *GetNameSafe(this),
		       *InputTag.ToString(),
		       *CurrentCombatStyleTag.ToString());
	}

	return bActivated;
}

UAMBCombatStyleData* AAMBCharacter::GetConfiguredCombatStyle(EAMBCombatStyleType CombatStyleType) const
{
	switch (CombatStyleType)
	{
	case EAMBCombatStyleType::Unarmed:
		return UnarmedCombatStyle;
	case EAMBCombatStyleType::Sword:
		return SwordCombatStyle;
	case EAMBCombatStyleType::Bow:
		return BowCombatStyle;
	default:
		return nullptr;
	}
}

UAMBCombatStyleData* AAMBCharacter::GetConfiguredCombatSlotStyle(int32 SlotIndex) const
{
	switch (SlotIndex)
	{
	case 1:
		return CombatSlot1Style;
	case 2:
		return CombatSlot2Style;
	default:
		return nullptr;
	}
}

EAMBCombatStyleType AAMBCharacter::ResolveCombatStyleType(const UAMBCombatStyleData* CombatStyleData) const
{
	if (!CombatStyleData)
	{
		return EAMBCombatStyleType::Unarmed;
	}

	if (CombatStyleData->CombatStyleType != EAMBCombatStyleType::Unarmed || CombatStyleData == UnarmedCombatStyle)
	{
		return CombatStyleData->CombatStyleType;
	}

	if (CombatStyleData == SwordCombatStyle)
	{
		return EAMBCombatStyleType::Sword;
	}

	if (CombatStyleData == BowCombatStyle)
	{
		return EAMBCombatStyleType::Bow;
	}

	return EAMBCombatStyleType::Unarmed;
}

UAMBItemData* AAMBCharacter::GetSelectedItemData() const
{
	return InventoryComponent ? InventoryComponent->GetSelectedItem() : nullptr;
}

float AAMBCharacter::GetCurrentEquippedItemBaseDamage() const
{
	const UAMBItemData* ItemData = GetSelectedItemData();
	return ItemData ? ItemData->BaseDamage : 0.0f;
}

float AAMBCharacter::GetCurrentWeaponDamage() const
{
	const float BaseDamage = GetCurrentEquippedItemBaseDamage();
	const float AttackPower = CombatAttributeSet ? CombatAttributeSet->GetAttackPower() : 1.0f;
	return FMath::Max(0.0f, BaseDamage * AttackPower);
}

bool AAMBCharacter::ApplyCurrentWeaponDamageToTarget(AActor* TargetActor, FVector DamageLocation, FVector DamageImpulse)
{
	static_cast<void>(DamageLocation);
	static_cast<void>(DamageImpulse);

	if (!TargetActor || TargetActor == this)
	{
		return false;
	}

	const float DamageAmount = GetCurrentWeaponDamage();
	if (DamageAmount <= 0.0f)
	{
		return false;
	}

	return ApplyDamageToTarget(TargetActor, DamageAmount, this, this);
}

void AAMBCharacter::DoComboAttackStart()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = TAG_Event_Attack_Combo_Input;
	EventPayload.Instigator = this;
	EventPayload.Target = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Attack_Combo_Input, EventPayload);

	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(TAG_State_Attack_Combo_Active))
	{
		return;
	}

	TryActivateCombatAbilityByInputTag(TAG_Input_Attack_Light);
}

void AAMBCharacter::DoComboAttackEnd()
{
	UE_LOG(LogMudAndBlood, Verbose,
	       TEXT("%s received DoComboAttackEnd, but combo end is not routed directly outside GAS."), *GetNameSafe(this));
}

void AAMBCharacter::DoChargedAttackStart()
{
	TryActivateCombatAbilityByInputTag(TAG_Input_Attack_Heavy_Start);
}

void AAMBCharacter::DoChargedAttackEnd()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = TAG_Event_Attack_Charged_Release;
	EventPayload.Instigator = this;
	EventPayload.Target = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Attack_Charged_Release, EventPayload);
}

void AAMBCharacter::SetCombatStyle(UAMBCombatStyleData* NewCombatStyle)
{
	if (CurrentCombatStyle == NewCombatStyle)
	{
		return;
	}

	ClearGrantedCombatAbilities();

	CurrentCombatStyle = NewCombatStyle;
	UpdateCombatStyleTag(CurrentCombatStyle ? CurrentCombatStyle->CombatStyleTag : FGameplayTag());
	CurrentCombatStyleType = ResolveCombatStyleType(CurrentCombatStyle);

	if (CombatAttackComponent && CurrentCombatStyle)
	{
		CombatAttackComponent->ApplyCombatStyleData(CurrentCombatStyle);
	}

	GrantCombatStyleAbilities(CurrentCombatStyle);

	UE_LOG(LogMudAndBlood, Log, TEXT("%s switched combat style to %s (Tag=%s)."),
	       *GetNameSafe(this),
	       *GetNameSafe(CurrentCombatStyle),
	       *CurrentCombatStyleTag.ToString());

	OnCombatStyleChanged.Broadcast(CurrentCombatSlotIndex, CurrentCombatStyleType, CurrentCombatStyle);
}

void AAMBCharacter::SetDefaultCombatStyle(UAMBCombatStyleData* NewDefaultCombatStyle, int32 SourceSlotIndex)
{
	const bool bSlotChanged = SourceSlotIndex != INDEX_NONE && CurrentCombatSlotIndex != SourceSlotIndex;
	const bool bStyleChanged = CurrentCombatStyle != NewDefaultCombatStyle;

	DefaultCombatStyle = NewDefaultCombatStyle;

	if (SourceSlotIndex != INDEX_NONE)
	{
		CurrentCombatSlotIndex = SourceSlotIndex;
	}

	SetCombatStyle(NewDefaultCombatStyle);

	if (!bStyleChanged && bSlotChanged)
	{
		OnCombatStyleChanged.Broadcast(CurrentCombatSlotIndex, CurrentCombatStyleType, CurrentCombatStyle);
	}
}

void AAMBCharacter::UpdateEquippedItemMesh(UAMBItemData* ItemData)
{
	if (!EquippedItemMeshComponent)
	{
		return;
	}

	UStaticMesh* EquippedMesh = ItemData ? ItemData->EquippedMesh.Get() : nullptr;
	if (!EquippedMesh)
	{
		UE_LOG(LogMudAndBlood, Log,
		       TEXT("%s cleared equipped item mesh. Item=%s"),
		       *GetNameSafe(this),
		       *GetNameSafe(ItemData));

		EquippedItemMeshComponent->SetStaticMesh(nullptr);
		EquippedItemMeshComponent->SetHiddenInGame(true);
		return;
	}

	USceneComponent* AttachComponent = GetEquippedItemAttachComponent();
	if (!AttachComponent)
	{
		EquippedItemMeshComponent->SetStaticMesh(nullptr);
		EquippedItemMeshComponent->SetHiddenInGame(true);
		return;
	}

	EquippedItemMeshComponent->SetStaticMesh(EquippedMesh);
	EquippedItemMeshComponent->AttachToComponent(
		AttachComponent,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		ItemData->EquippedMeshSocketName
	);
	EquippedItemMeshComponent->SetRelativeLocation(ItemData->EquippedMeshRelativeLocation);
	EquippedItemMeshComponent->SetRelativeRotation(ItemData->EquippedMeshRelativeRotation);
	EquippedItemMeshComponent->SetRelativeScale3D(ItemData->EquippedMeshRelativeScale);
	EquippedItemMeshComponent->SetHiddenInGame(false);

	UE_LOG(LogMudAndBlood, Log,
	       TEXT("%s equipped item mesh updated. Item=%s Mesh=%s AttachSocket=%s SelectedSlot=%d"),
	       *GetNameSafe(this),
	       *GetNameSafe(ItemData),
	       *GetNameSafe(EquippedMesh),
	       *ItemData->EquippedMeshSocketName.ToString(),
	       InventoryComponent ? InventoryComponent->GetSelectedSlotIndex() : INDEX_NONE);
}

void AAMBCharacter::EquipCombatStyleByType(EAMBCombatStyleType CombatStyleType)
{
	UAMBCombatStyleData* CombatStyle = GetConfiguredCombatStyle(CombatStyleType);
	if (!CombatStyle)
	{
		static const UEnum* CombatStyleEnum = StaticEnum<EAMBCombatStyleType>();
		const FString CombatStyleName = CombatStyleEnum
			                                ? CombatStyleEnum->GetNameStringByValue(static_cast<int64>(CombatStyleType))
			                                : TEXT("Unknown");

		UE_LOG(LogMudAndBlood, Warning,
		       TEXT("%s cannot equip combat style %s: no style asset is assigned on the character."),
		       *GetNameSafe(this),
		       *CombatStyleName);
		return;
	}

	SetCombatStyle(CombatStyle);
}

void AAMBCharacter::SetAttackType(EAMBCombatStyleType CombatStyleType)
{
	EquipCombatStyleByType(CombatStyleType);
}

void AAMBCharacter::EquipCombatSlot(int32 SlotIndex)
{
	UAMBCombatStyleData* CombatStyle = GetConfiguredCombatSlotStyle(SlotIndex);
	if (!CombatStyle)
	{
		UE_LOG(LogMudAndBlood, Warning,
		       TEXT("%s cannot equip combat slot %d: no UAMBCombatStyleData is assigned to that slot."),
		       *GetNameSafe(this),
		       SlotIndex);
		return;
	}

	CurrentCombatSlotIndex = SlotIndex;

	UE_LOG(LogMudAndBlood, Log, TEXT("%s selected combat slot %d -> %s."),
	       *GetNameSafe(this),
	       SlotIndex,
	       *GetNameSafe(CombatStyle));

	SetDefaultCombatStyle(CombatStyle, SlotIndex);
}

void AAMBCharacter::HandleInventorySlotSelected(int32 SlotIndex, UAMBItemData* ItemData)
{
	static_cast<void>(SlotIndex);
	UpdateEquippedItemMesh(ItemData);
}

void AAMBCharacter::DoAttackTrace(FName TraceStartBone, FName TraceEndBone)
{
	AActor* HitActor = nullptr;
	FVector ImpactPoint = FVector::ZeroVector;
	SphereTraceMultiForObjects(TraceStartBone, TraceEndBone, HitActor, ImpactPoint);
}

bool AAMBCharacter::SphereTraceMultiForObjects(FName TraceStartBone, FName TraceEndBone, AActor*& HitActor, FVector& ImpactPoint)
{
	HitActor = nullptr;
	ImpactPoint = FVector::ZeroVector;

	if (CombatAttackComponent)
	{
		return CombatAttackComponent->AttackSphereTrace(TraceStartBone, TraceEndBone, HitActor, ImpactPoint);
	}

	return false;
}

void AAMBCharacter::BeginAttackTraceWindow(FName TraceStartBone, FName TraceEndBone)
{
	if (CombatAttackComponent)
	{
		CombatAttackComponent->BeginAttackTraceWindow(TraceStartBone, TraceEndBone);
	}
}

void AAMBCharacter::TickAttackTraceWindow(FName TraceStartBone, FName TraceEndBone)
{
	if (CombatAttackComponent)
	{
		CombatAttackComponent->TickAttackTraceWindow(TraceStartBone, TraceEndBone);
	}
}

void AAMBCharacter::EndAttackTraceWindow()
{
	if (CombatAttackComponent)
	{
		CombatAttackComponent->EndAttackTraceWindow();
	}
}

void AAMBCharacter::CheckCombo()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = TAG_Event_Attack_Combo_Check;
	EventPayload.Instigator = this;
	EventPayload.Target = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Attack_Combo_Check, EventPayload);
}

void AAMBCharacter::CheckChargedAttack()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = TAG_Event_Attack_Charged_Check;
	EventPayload.Instigator = this;
	EventPayload.Target = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Attack_Charged_Check, EventPayload);
}

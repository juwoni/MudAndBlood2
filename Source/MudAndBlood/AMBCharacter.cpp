// Fill out your copyright notice in the Description page of Project Settings.


#include "AMBCharacter.h"
#include "AbilitySystem/AMBAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AMBCombatAttributeSet.h"
#include "AbilitySystem/AMBGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Characters/Components/AMBCombatStyleComponent.h"
#include "Characters/Components/AMBEquipmentVisualComponent.h"
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
	EquippedItemMeshComponent = CreateDefaultSubobject<UAMBEquipmentVisualComponent>(TEXT("EquippedItemMeshComponent"));
	CombatStyleComponent = CreateDefaultSubobject<UAMBCombatStyleComponent>(TEXT("CombatStyleComponent"));
	EquippedItemMeshComponent->SetupAttachment(GetMesh());
}

// Called when the game starts or when spawned
void AAMBCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CombatStyleComponent)
	{
		CombatStyleComponent->OnCombatStyleChanged.AddDynamic(this, &AAMBCharacter::HandleCombatStyleChanged);
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

	if (CombatStyleComponent)
	{
		CombatStyleComponent->TryActivateCombatAbilityByInputTag(TAG_Input_Attack_Light);
	}
}

void AAMBCharacter::DoComboAttackEnd()
{
	UE_LOG(LogMudAndBlood, Verbose,
	       TEXT("%s received DoComboAttackEnd, but combo end is not routed directly outside GAS."), *GetNameSafe(this));
}

void AAMBCharacter::DoChargedAttackStart()
{
	if (CombatStyleComponent)
	{
		CombatStyleComponent->TryActivateCombatAbilityByInputTag(TAG_Input_Attack_Heavy_Start);
	}
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
	if (CombatStyleComponent)
	{
		CombatStyleComponent->SetCombatStyle(NewCombatStyle);
	}
}

void AAMBCharacter::SetDefaultCombatStyle(UAMBCombatStyleData* NewDefaultCombatStyle, int32 SourceSlotIndex)
{
	if (CombatStyleComponent)
	{
		CombatStyleComponent->SetDefaultCombatStyle(NewDefaultCombatStyle, SourceSlotIndex);
	}
}

void AAMBCharacter::EquipCombatStyleByType(EAMBCombatStyleType CombatStyleType)
{
	if (CombatStyleComponent)
	{
		CombatStyleComponent->EquipCombatStyleByType(CombatStyleType);
	}
}

void AAMBCharacter::SetAttackType(EAMBCombatStyleType CombatStyleType)
{
	EquipCombatStyleByType(CombatStyleType);
}

void AAMBCharacter::EquipCombatSlot(int32 SlotIndex)
{
	if (InventoryComponent)
	{
		InventoryComponent->SelectInventorySlot(SlotIndex);
	}
}

void AAMBCharacter::HandleCombatStyleChanged(int32 SlotIndex, EAMBCombatStyleType CombatStyleType,
                                             UAMBCombatStyleData* CombatStyleData)
{
	OnCombatStyleChanged.Broadcast(SlotIndex, CombatStyleType, CombatStyleData);
}

FGameplayTag AAMBCharacter::GetCurrentCombatStyleTag() const
{
	return CombatStyleComponent ? CombatStyleComponent->GetCurrentCombatStyleTag() : FGameplayTag();
}

EAMBCombatStyleType AAMBCharacter::GetCurrentCombatStyleType() const
{
	return CombatStyleComponent ? CombatStyleComponent->GetCurrentCombatStyleType() : EAMBCombatStyleType::Unarmed;
}

int32 AAMBCharacter::GetCurrentCombatSlotIndex() const
{
	return CombatStyleComponent ? CombatStyleComponent->GetCurrentCombatSlotIndex() : INDEX_NONE;
}

UAMBCombatStyleData* AAMBCharacter::GetCurrentCombatStyle() const
{
	return CombatStyleComponent ? CombatStyleComponent->GetCurrentCombatStyle() : nullptr;
}

UStaticMeshComponent* AAMBCharacter::GetEquippedWeaponMesh() const
{
	return EquippedItemMeshComponent;
}

UStaticMeshComponent* AAMBCharacter::GetEquippedItemMeshComponent() const
{
	return EquippedItemMeshComponent;
}

void AAMBCharacter::DoAttackTrace(FName TraceStartBone, FName TraceEndBone)
{
	if (CombatAttackComponent)
	{
		CombatAttackComponent->DoAttackTrace(TraceStartBone, TraceEndBone);
	}
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

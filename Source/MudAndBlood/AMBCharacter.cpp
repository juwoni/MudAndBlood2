// Fill out your copyright notice in the Description page of Project Settings.


#include "AMBCharacter.h"
#include "AbilitySystem/AMBAbilitySystemComponent.h"
#include "AbilitySystem/AMBCombatAttributeSet.h"
#include "AbilitySystem/AMBGameplayTags.h"
#include "EnhancedInputComponent.h"
#include "MudAndBlood.h"
#include "Variant_Combat/Components/CombatAttackComponent.h"
#include "Variant_Combat/Data/AMBCombatStyleData.h"

// Sets default values
AAMBCharacter::AAMBCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAMBAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	CombatAttributeSet = CreateDefaultSubobject<UAMBCombatAttributeSet>(TEXT("CombatAttributeSet"));
	CombatAttackComponent = CreateDefaultSubobject<UCombatAttackComponent>(TEXT("CombatAttackComponent"));
}

UAbilitySystemComponent* AAMBCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Called when the game starts or when spawned
void AAMBCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilityActorInfo();

	if (DefaultCombatStyle)
	{
		SetCombatStyle(DefaultCombatStyle);
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
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this, &AAMBCharacter::ComboAttackPressed);
		EnhancedInputComponent->BindAction(ChargedAttackAction, ETriggerEvent::Started, this, &AAMBCharacter::ChargedAttackPressed);
		EnhancedInputComponent->BindAction(ChargedAttackAction, ETriggerEvent::Completed, this, &AAMBCharacter::ChargedAttackReleased);
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

void AAMBCharacter::InitializeAbilityActorInfo()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
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
		UE_LOG(LogMudAndBlood, Warning, TEXT("%s failed to activate combat input %s: AbilitySystemComponent is missing."),
			*GetNameSafe(this),
			*InputTag.ToString());
		return false;
	}

	if (!AbilitySystemComponent->HasAbilityWithInputTag(InputTag))
	{
		UE_LOG(LogMudAndBlood, Warning, TEXT("%s failed to activate combat input %s: no granted ability matches this input tag. CurrentStyle=%s"),
			*GetNameSafe(this),
			*InputTag.ToString(),
			*CurrentCombatStyleTag.ToString());
		return false;
	}

	const bool bActivated = AbilitySystemComponent->TryActivateAbilitiesByInputTag(InputTag);
	if (!bActivated)
	{
		UE_LOG(LogMudAndBlood, Warning, TEXT("%s failed to activate combat input %s: matching ability exists but activation was rejected. CurrentStyle=%s"),
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

void AAMBCharacter::DoComboAttackStart()
{
	TryActivateCombatAbilityByInputTag(TAG_Input_Attack_Light);
}

void AAMBCharacter::DoComboAttackEnd()
{
	UE_LOG(LogMudAndBlood, Verbose, TEXT("%s received DoComboAttackEnd, but combo end is not routed directly outside GAS."), *GetNameSafe(this));
}

void AAMBCharacter::DoChargedAttackStart()
{
	TryActivateCombatAbilityByInputTag(TAG_Input_Attack_Heavy_Start);
}

void AAMBCharacter::DoChargedAttackEnd()
{
	TryActivateCombatAbilityByInputTag(TAG_Input_Attack_Heavy_Release);
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

	if (CombatAttackComponent && CurrentCombatStyle)
	{
		CombatAttackComponent->ApplyCombatStyleData(CurrentCombatStyle);
	}

	GrantCombatStyleAbilities(CurrentCombatStyle);

	UE_LOG(LogMudAndBlood, Log, TEXT("%s switched combat style to %s (Tag=%s)."),
		*GetNameSafe(this),
		*GetNameSafe(CurrentCombatStyle),
		*CurrentCombatStyleTag.ToString());
}

void AAMBCharacter::EquipCombatStyleByType(EAMBCombatStyleType CombatStyleType)
{
	UAMBCombatStyleData* CombatStyle = GetConfiguredCombatStyle(CombatStyleType);
	if (!CombatStyle)
	{
		static const UEnum* CombatStyleEnum = StaticEnum<EAMBCombatStyleType>();
		const FString CombatStyleName = CombatStyleEnum ? CombatStyleEnum->GetNameStringByValue(static_cast<int64>(CombatStyleType)) : TEXT("Unknown");

		UE_LOG(LogMudAndBlood, Warning, TEXT("%s cannot equip combat style %s: no style asset is assigned on the character."),
			*GetNameSafe(this),
			*CombatStyleName);
		return;
	}

	SetCombatStyle(CombatStyle);
}

void AAMBCharacter::DoAttackTrace(FName DamageSourceBone)
{
	if (CombatAttackComponent)
	{
		CombatAttackComponent->DoAttackTrace(DamageSourceBone);
	}
}

void AAMBCharacter::CheckCombo()
{
	if (CombatAttackComponent)
	{
		CombatAttackComponent->CheckCombo();
	}
}

void AAMBCharacter::CheckChargedAttack()
{
	if (CombatAttackComponent)
	{
		CombatAttackComponent->CheckChargedAttack();
	}
}

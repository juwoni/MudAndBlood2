// Fill out your copyright notice in the Description page of Project Settings.


#include "AMBCharacter.h"
#include "AbilitySystem/AMBAbilitySystemComponent.h"
#include "AbilitySystem/AMBCombatAttributeSet.h"
#include "AbilitySystem/AMBGameplayTags.h"
#include "EnhancedInputComponent.h"
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
	if (!AbilitySystemComponent || !AbilitySystemComponent->HasAbilityWithInputTag(InputTag))
	{
		return false;
	}

	AbilitySystemComponent->TryActivateAbilitiesByInputTag(InputTag);
	return true;
}

void AAMBCharacter::DoComboAttackStart()
{
	if (TryActivateCombatAbilityByInputTag(TAG_Input_Attack_Light))
	{
		return;
	}

	if (CombatAttackComponent)
	{
		CombatAttackComponent->DoComboAttackStart();
	}
}

void AAMBCharacter::DoComboAttackEnd()
{
	if (CombatAttackComponent)
	{
		CombatAttackComponent->DoComboAttackEnd();
	}
}

void AAMBCharacter::DoChargedAttackStart()
{
	if (TryActivateCombatAbilityByInputTag(TAG_Input_Attack_Heavy_Start))
	{
		return;
	}

	if (CombatAttackComponent)
	{
		CombatAttackComponent->DoChargedAttackStart();
	}
}

void AAMBCharacter::DoChargedAttackEnd()
{
	if (TryActivateCombatAbilityByInputTag(TAG_Input_Attack_Heavy_Release))
	{
		return;
	}

	if (CombatAttackComponent)
	{
		CombatAttackComponent->DoChargedAttackEnd();
	}
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

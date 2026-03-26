// Fill out your copyright notice in the Description page of Project Settings.


#include "AMBCharacter.h"
#include "Variant_Combat/Components/CombatAttackComponent.h"
#include "EnhancedInputComponent.h"

// Sets default values
AAMBCharacter::AAMBCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CombatAttackComponent = CreateDefaultSubobject<UCombatAttackComponent>(TEXT("CombatAttackComponent"));
}

// Called when the game starts or when spawned
void AAMBCharacter::BeginPlay()
{
	Super::BeginPlay();
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

void AAMBCharacter::DoComboAttackStart()
{
	
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
	if (CombatAttackComponent)
	{
		CombatAttackComponent->DoChargedAttackStart();
	}
}

void AAMBCharacter::DoChargedAttackEnd()
{
	if (CombatAttackComponent)
	{
		CombatAttackComponent->DoChargedAttackEnd();
	}
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


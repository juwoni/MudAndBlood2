// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Components/DummyDamageableComponent.h"
#include "DummyCharacter.h"

#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ADummyCharacter::ADummyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DummyDamageableComponent = CreateDefaultSubobject<UDummyDamageableComponent>(TEXT("DummyDamageableComponent"));
}

// Called when the game starts or when spawned
void ADummyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (DummyDamageableComponent)
	{
		DummyDamageableComponent->OnDamaged.AddDynamic(this, &ADummyCharacter::HandleDummyDamaged);
		DummyDamageableComponent->OnDeath.AddDynamic(this, &ADummyCharacter::HandleDummyDied);
	}
}

// Called every frame
void ADummyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ADummyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ADummyCharacter::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{
	if (!DummyDamageableComponent)
	{
		return;
	}

	FDamageEvent DamageEvent;
	Super::TakeDamage(Damage, DamageEvent, DamageCauser ? DamageCauser->GetInstigatorController() : nullptr, DamageCauser);

	const float ActualDamage = DummyDamageableComponent->ApplyDamage(Damage, DamageCauser, DamageLocation, DamageImpulse);
	if (ActualDamage <= 0.0f)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->AddImpulse(DamageImpulse, true);
	}
}

void ADummyCharacter::HandleDeath()
{
	if (DummyDamageableComponent)
	{
		DummyDamageableComponent->HandleDeath(nullptr);
	}
}

void ADummyCharacter::ApplyHealing(float Healing, AActor* Healer)
{
	if (DummyDamageableComponent)
	{
		DummyDamageableComponent->ApplyHealing(Healing, Healer);
	}
}

void ADummyCharacter::NotifyDanger(const FVector& DangerLocation, AActor* DangerSource)
{
}

float ADummyCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (!DummyDamageableComponent)
	{
		return 0.0f;
	}

	return DummyDamageableComponent->ApplyDamage(ActualDamage, DamageCauser, FVector::ZeroVector, FVector::ZeroVector);
}

void ADummyCharacter::HandleDummyDamaged(float Damage, float CurrentHealth, AActor* DamageCauser, FVector DamageLocation, FVector DamageImpulse)
{
	BP_OnDummyDamaged(Damage, CurrentHealth, DamageLocation, DamageImpulse.GetSafeNormal(), DamageCauser);
}

void ADummyCharacter::HandleDummyDied(AActor* DamageCauser)
{
	BP_OnDummyDied(DamageCauser);
}

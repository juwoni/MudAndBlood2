#include "Characters/Components/DummyDamageableComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UDummyDamageableComponent::UDummyDamageableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDummyDamageableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bResetHealthOnBeginPlay)
	{
		ResetHealth();
	}
	else
	{
		CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
		bIsDead = CurrentHealth <= 0.0f;
	}
}

float UDummyDamageableComponent::ApplyDamage(float Damage, AActor* DamageCauser, FVector DamageLocation, FVector DamageImpulse)
{
	if (Damage <= 0.0f || bIsDead)
	{
		return 0.0f;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);

	const float ActualDamage = PreviousHealth - CurrentHealth;
	if (ActualDamage <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	OnDamaged.Broadcast(ActualDamage, CurrentHealth, DamageCauser, DamageLocation, DamageImpulse);

	if (CurrentHealth <= 0.0f)
	{
		HandleDeath(DamageCauser);
	}

	return ActualDamage;
}

float UDummyDamageableComponent::ApplyHealing(float Healing, AActor* Healer)
{
	if (Healing <= 0.0f || MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + Healing, 0.0f, MaxHealth);
	bIsDead = CurrentHealth <= 0.0f;

	const float ActualHealing = CurrentHealth - PreviousHealth;
	if (ActualHealing > KINDA_SMALL_NUMBER)
	{
		OnHealed.Broadcast(ActualHealing, CurrentHealth, Healer);
	}

	return ActualHealing;
}

void UDummyDamageableComponent::ResetHealth()
{
	CurrentHealth = FMath::Max(0.0f, MaxHealth);
	bIsDead = CurrentHealth <= 0.0f;
}

void UDummyDamageableComponent::HandleDeath(AActor* DamageCauser)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	CurrentHealth = 0.0f;

	if (ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* CharacterMovement = CharacterOwner->GetCharacterMovement())
		{
			CharacterMovement->StopMovementImmediately();
			CharacterMovement->DisableMovement();
		}
	}

	if (AActor* OwnerActor = GetOwner())
	{
		if (bDisableCollisionOnDeath)
		{
			OwnerActor->SetActorEnableCollision(false);
		}

		OnDeath.Broadcast(DamageCauser);

		if (bDestroyOwnerOnDeath)
		{
			OwnerActor->Destroy();
		}
	}
}

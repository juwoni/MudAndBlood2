// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatEnemy.h"

#include "AbilitySystem/AMBAbilitySystemComponent.h"
#include "AbilitySystem/AMBGameplayTags.h"
#include "AbilitySystem/Attributes/AMBCombatAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CombatAIController.h"
#include "CombatLifeBar.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Variant_Combat/Components/CombatAttackComponent.h"
#include "Variant_Combat/Data/AMBCombatStyleData.h"

ACombatEnemy::ACombatEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = ACombatAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;

	LifeBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("LifeBar"));
	LifeBar->SetupAttachment(RootComponent);

	CombatAttackComponent = CreateDefaultSubobject<UCombatAttackComponent>(TEXT("CombatAttackComponent"));

	GetCapsuleComponent()->SetCapsuleSize(35.0f, 90.0f);
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	CurrentHP = MaxHP;
}

void ACombatEnemy::DoAIComboAttack()
{
	if (bIsAttacking || !CurrentCombatStyle)
	{
		return;
	}

	const int32 ComboSectionCount = CurrentCombatStyle->ComboSectionNames.Num();
	if (ComboSectionCount <= 0 || !CurrentCombatStyle->ComboAttackMontage)
	{
		return;
	}

	bIsAttacking = true;
	CurrentComboAttack = 0;
	TargetComboCount = FMath::RandRange(1, ComboSectionCount);

	if (AbilitySystemComponent)
	{
		if (ComboStateTagChangedHandle.IsValid())
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(TAG_State_Attack_Combo_Active, EGameplayTagEventType::NewOrRemoved)
				.Remove(ComboStateTagChangedHandle);
			ComboStateTagChangedHandle.Reset();
		}

		ComboStateTagChangedHandle = AbilitySystemComponent->RegisterGameplayTagEvent(
			TAG_State_Attack_Combo_Active,
			EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ACombatEnemy::HandleComboAbilityStateChanged);
	}

	if (!TryActivateCombatAbilityByInputTag(TAG_Input_Attack_Light))
	{
		if (AbilitySystemComponent && ComboStateTagChangedHandle.IsValid())
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(TAG_State_Attack_Combo_Active, EGameplayTagEventType::NewOrRemoved)
				.Remove(ComboStateTagChangedHandle);
			ComboStateTagChangedHandle.Reset();
		}

		NotifyAttackCompleted();
	}
}

void ACombatEnemy::DoAIChargedAttack()
{
	if (bIsAttacking || !CurrentCombatStyle)
	{
		return;
	}

	if (!CurrentCombatStyle->ChargedAttackMontage)
	{
		return;
	}

	bIsAttacking = true;
	CurrentChargeLoop = 0;
	TargetChargeLoops = FMath::RandRange(MinChargeLoops, MaxChargeLoops);

	if (AbilitySystemComponent)
	{
		if (ChargedStateTagChangedHandle.IsValid())
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(TAG_State_Attack_Charged_Active, EGameplayTagEventType::NewOrRemoved)
				.Remove(ChargedStateTagChangedHandle);
			ChargedStateTagChangedHandle.Reset();
		}

		ChargedStateTagChangedHandle = AbilitySystemComponent->RegisterGameplayTagEvent(
			TAG_State_Attack_Charged_Active,
			EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ACombatEnemy::HandleChargedAbilityStateChanged);
	}

	if (!TryActivateCombatAbilityByInputTag(TAG_Input_Attack_Heavy_Start))
	{
		if (AbilitySystemComponent && ChargedStateTagChangedHandle.IsValid())
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(TAG_State_Attack_Charged_Active, EGameplayTagEventType::NewOrRemoved)
				.Remove(ChargedStateTagChangedHandle);
			ChargedStateTagChangedHandle.Reset();
		}

		NotifyAttackCompleted();
	}
}

const FVector& ACombatEnemy::GetLastDangerLocation() const
{
	return LastDangerLocation;
}

float ACombatEnemy::GetLastDangerTime() const
{
	return LastDangerTime;
}

void ACombatEnemy::PrepareAttackTrace()
{
	if (!CombatAttackComponent)
	{
		return;
	}

	if (CurrentCombatStyle)
	{
		CombatAttackComponent->ApplyCombatStyleData(CurrentCombatStyle);
		return;
	}

	CombatAttackComponent->SetMeleeTraceSettings(
		MeleeTraceDistance,
		MeleeTraceRadius,
		MeleeDamage,
		MeleeKnockbackImpulse,
		MeleeLaunchImpulse);
}

void ACombatEnemy::CheckCombo()
{
	++CurrentComboAttack;

	if (CurrentComboAttack < TargetComboCount)
	{
		FGameplayEventData ComboInputPayload;
		ComboInputPayload.EventTag = TAG_Event_Attack_Combo_Input;
		ComboInputPayload.Instigator = this;
		ComboInputPayload.Target = this;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Attack_Combo_Input, ComboInputPayload);
	}

	FGameplayEventData CheckPayload;
	CheckPayload.EventTag = TAG_Event_Attack_Combo_Check;
	CheckPayload.Instigator = this;
	CheckPayload.Target = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Attack_Combo_Check, CheckPayload);
}

void ACombatEnemy::CheckChargedAttack()
{
	++CurrentChargeLoop;

	if (CurrentChargeLoop >= TargetChargeLoops)
	{
		FGameplayEventData ReleasePayload;
		ReleasePayload.EventTag = TAG_Event_Attack_Charged_Release;
		ReleasePayload.Instigator = this;
		ReleasePayload.Target = this;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Attack_Charged_Release, ReleasePayload);
	}

	FGameplayEventData CheckPayload;
	CheckPayload.EventTag = TAG_Event_Attack_Charged_Check;
	CheckPayload.Instigator = this;
	CheckPayload.Target = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Attack_Charged_Check, CheckPayload);
}

void ACombatEnemy::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation,
                               const FVector& DamageImpulse)
{
	const float OldHealth = GetHealth();
	Super::ApplyDamage(Damage, DamageCauser, DamageLocation, DamageImpulse);

	if (GetHealth() >= OldHealth)
	{
		return;
	}

	if (GetMesh()->IsSimulatingPhysics())
	{
		GetMesh()->AddImpulseAtLocation(DamageImpulse * GetMesh()->GetMass(), DamageLocation);
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (CurrentCombatStyle)
		{
			AnimInstance->Montage_Stop(0.1f, CurrentCombatStyle->ComboAttackMontage);
			AnimInstance->Montage_Stop(0.1f, CurrentCombatStyle->ChargedAttackMontage);
		}
		else
		{
			AnimInstance->Montage_Stop(0.1f, ComboAttackMontage);
			AnimInstance->Montage_Stop(0.1f, ChargedAttackMontage);
		}
	}

	ReceivedDamage(OldHealth - GetHealth(), DamageLocation, DamageImpulse.GetSafeNormal());
}

void ACombatEnemy::HandleDeath()
{
	Super::HandleDeath();

	if (LifeBar)
	{
		LifeBar->SetHiddenInGame(true);
	}

	OnEnemyDied.Broadcast();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DeathTimer, this, &ACombatEnemy::RemoveFromLevel, DeathRemovalTime);
	}
}

void ACombatEnemy::ApplyHealing(float Healing, AActor* Healer)
{
	Super::ApplyHealing(Healing, Healer);
}

void ACombatEnemy::NotifyDanger(const FVector& DangerLocation, AActor* DangerSource)
{
	if (DangerSource && DangerSource->ActorHasTag(FName("Player")))
	{
		LastDangerLocation = DangerLocation;
		LastDangerTime = GetWorld()->GetTimeSeconds();
	}
}

void ACombatEnemy::RemoveFromLevel()
{
	Destroy();
}

float ACombatEnemy::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator,
                               AActor* DamageCauser)
{
	static_cast<void>(DamageEvent);
	static_cast<void>(EventInstigator);

	if (IsDead())
	{
		return 0.0f;
	}

	return ApplyDamageToSelf(Damage, DamageCauser, DamageCauser) ? FMath::Abs(Damage) : 0.0f;
}

void ACombatEnemy::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (!IsDead())
	{
		GetMesh()->SetPhysicsBlendWeight(0.0f);
	}

	OnEnemyLanded.ExecuteIfBound();
}

void ACombatEnemy::BeginPlay()
{
	if (CombatAttributeSet)
	{
		CombatAttributeSet->SetMaxHealth(MaxHP);
		CombatAttributeSet->SetHealth(MaxHP);
	}

	Super::BeginPlay();

	LifeBarWidget = Cast<UCombatLifeBar>(LifeBar->GetUserWidgetObject());
	check(LifeBarWidget);

	CurrentHP = GetHealth();
	LifeBarWidget->SetLifePercentage(GetMaxHealth() > 0.0f ? GetHealth() / GetMaxHealth() : 0.0f);

	if (CombatStyle)
	{
		SetCombatStyle(CombatStyle);
	}
}

void ACombatEnemy::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (AbilitySystemComponent)
	{
		if (ComboStateTagChangedHandle.IsValid())
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(TAG_State_Attack_Combo_Active, EGameplayTagEventType::NewOrRemoved)
				.Remove(ComboStateTagChangedHandle);
			ComboStateTagChangedHandle.Reset();
		}

		if (ChargedStateTagChangedHandle.IsValid())
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(TAG_State_Attack_Charged_Active, EGameplayTagEventType::NewOrRemoved)
				.Remove(ChargedStateTagChangedHandle);
			ChargedStateTagChangedHandle.Reset();
		}
	}

	Super::EndPlay(EndPlayReason);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeathTimer);
	}
}

void ACombatEnemy::ClearGrantedCombatAbilities()
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

void ACombatEnemy::GrantCombatStyleAbilities(const UAMBCombatStyleData* CombatStyleData)
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

void ACombatEnemy::UpdateCombatStyleTag(const FGameplayTag& NewCombatStyleTag)
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

bool ACombatEnemy::TryActivateCombatAbilityByInputTag(const FGameplayTag& InputTag) const
{
	if (!AbilitySystemComponent)
	{
		return false;
	}

	return AbilitySystemComponent->TryActivateAbilitiesByInputTag(InputTag);
}

void ACombatEnemy::SetCombatStyle(UAMBCombatStyleData* NewCombatStyle)
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

		MeleeTraceDistance = CurrentCombatStyle->MeleeTraceDistance;
		MeleeTraceRadius = CurrentCombatStyle->MeleeTraceRadius;
		MeleeDamage = CurrentCombatStyle->MeleeDamage;
		MeleeKnockbackImpulse = CurrentCombatStyle->MeleeKnockbackImpulse;
		MeleeLaunchImpulse = CurrentCombatStyle->MeleeLaunchImpulse;
		ComboAttackMontage = CurrentCombatStyle->ComboAttackMontage;
		ComboSectionNames = CurrentCombatStyle->ComboSectionNames;
		ChargedAttackMontage = CurrentCombatStyle->ChargedAttackMontage;
		ChargeLoopSection = CurrentCombatStyle->ChargeLoopSection;
		ChargeAttackSection = CurrentCombatStyle->ChargeAttackSection;
	}

	GrantCombatStyleAbilities(CurrentCombatStyle);
}

void ACombatEnemy::HandleComboAbilityStateChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	static_cast<void>(CallbackTag);

	if (NewCount == 0 && bIsAttacking)
	{
		if (AbilitySystemComponent && ComboStateTagChangedHandle.IsValid())
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(TAG_State_Attack_Combo_Active, EGameplayTagEventType::NewOrRemoved)
				.Remove(ComboStateTagChangedHandle);
			ComboStateTagChangedHandle.Reset();
		}

		NotifyAttackCompleted();
	}
}

void ACombatEnemy::HandleChargedAbilityStateChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	static_cast<void>(CallbackTag);

	if (NewCount == 0 && bIsAttacking)
	{
		if (AbilitySystemComponent && ChargedStateTagChangedHandle.IsValid())
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(TAG_State_Attack_Charged_Active, EGameplayTagEventType::NewOrRemoved)
				.Remove(ChargedStateTagChangedHandle);
			ChargedStateTagChangedHandle.Reset();
		}

		NotifyAttackCompleted();
	}
}

void ACombatEnemy::NotifyAttackCompleted()
{
	bIsAttacking = false;
	OnAttackCompleted.ExecuteIfBound();
}

void ACombatEnemy::HandleHealthChanged(float OldHealth, float NewHealth, AActor* InstigatorActor)
{
	Super::HandleHealthChanged(OldHealth, NewHealth, InstigatorActor);

	CurrentHP = NewHealth;

	if (LifeBarWidget)
	{
		LifeBarWidget->SetLifePercentage(GetMaxHealth() > 0.0f ? NewHealth / GetMaxHealth() : 0.0f);
	}

	if (NewHealth < OldHealth && NewHealth > 0.0f)
	{
		GetMesh()->SetPhysicsBlendWeight(0.5f);
		GetMesh()->SetBodySimulatePhysics(PelvisBoneName, false);
	}
}

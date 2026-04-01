#include "Characters/AMBGASCharacterBase.h"

#include "AbilitySystem/AMBAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AMBCombatAttributeSet.h"
#include "AbilitySystem/AMBGameplayTags.h"
#include "AbilitySystem/Effects/AMBGameplayEffect_Damage.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffect.h"
#include "MudAndBlood.h"

AAMBGASCharacterBase::AAMBGASCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAMBAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->AddAttributeSetSubobject(CombatAttributeSet.Get());
	
	CombatAttributeSet = CreateDefaultSubobject<UAMBCombatAttributeSet>(TEXT("CombatAttributeSet"));
	DamageGameplayEffectClass = UAMBGameplayEffect_Damage::StaticClass();
}

UAbilitySystemComponent* AAMBGASCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float AAMBGASCharacterBase::GetHealth() const
{
	return CombatAttributeSet ? CombatAttributeSet->GetHealth() : 0.0f;
}

float AAMBGASCharacterBase::GetMaxHealth() const
{
	return CombatAttributeSet ? CombatAttributeSet->GetMaxHealth() : 0.0f;
}

bool AAMBGASCharacterBase::ApplyDamageToTarget(AActor* TargetActor, float DamageAmount, AActor* InstigatorActor, UObject* SourceObject)
{
	if (!TargetActor)
	{
		return false;
	}

	UAbilitySystemComponent* TargetAbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	return ApplyHealthDeltaToTarget(TargetAbilitySystem, -FMath::Abs(DamageAmount), InstigatorActor, SourceObject);
}

bool AAMBGASCharacterBase::ApplyDamageToSelf(float DamageAmount, AActor* InstigatorActor, UObject* SourceObject)
{
	return ApplyHealthDeltaToTarget(AbilitySystemComponent, -FMath::Abs(DamageAmount), InstigatorActor, SourceObject);
}

bool AAMBGASCharacterBase::ApplyHealingToSelf(float HealingAmount, AActor* Healer, UObject* SourceObject)
{
	return ApplyHealthDeltaToTarget(AbilitySystemComponent, FMath::Abs(HealingAmount), Healer, SourceObject);
}

void AAMBGASCharacterBase::DoAttackTrace(FName TraceStartBone, FName TraceEndBone)
{
	static_cast<void>(TraceStartBone);
	static_cast<void>(TraceEndBone);
}

void AAMBGASCharacterBase::BeginAttackTraceWindow(FName TraceStartBone, FName TraceEndBone)
{
	static_cast<void>(TraceStartBone);
	static_cast<void>(TraceEndBone);
}

void AAMBGASCharacterBase::TickAttackTraceWindow(FName TraceStartBone, FName TraceEndBone)
{
	static_cast<void>(TraceStartBone);
	static_cast<void>(TraceEndBone);
}

void AAMBGASCharacterBase::EndAttackTraceWindow()
{
}

void AAMBGASCharacterBase::CheckCombo()
{
}

void AAMBGASCharacterBase::CheckChargedAttack()
{
}

void AAMBGASCharacterBase::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{
	LastDamageCauser = DamageCauser;
	LastDamageLocation = DamageLocation;
	LastDamageImpulse = DamageImpulse;

	if (ApplyDamageToSelf(Damage, DamageCauser, DamageCauser))
	{
		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->AddImpulse(DamageImpulse, true);
		}
	}
}

void AAMBGASCharacterBase::HandleDeath()
{
	bIsDead = true;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	// GetMesh()->SetSimulatePhysics(true);
	HandleOutOfHealth(LastDamageCauser.Get());
}

void AAMBGASCharacterBase::ApplyHealing(float Healing, AActor* Healer)
{
	ApplyHealingToSelf(Healing, Healer, Healer);
}

void AAMBGASCharacterBase::NotifyDanger(const FVector& DangerLocation, AActor* DangerSource)
{
	static_cast<void>(DangerLocation);
	static_cast<void>(DangerSource);
}

void AAMBGASCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilityActorInfo();

	if (AbilitySystemComponent)
	{
		const UAMBCombatAttributeSet* RegisteredAttributeSet = Cast<const UAMBCombatAttributeSet>(
			AbilitySystemComponent->GetAttributeSet(UAMBCombatAttributeSet::StaticClass()));
		CombatAttributeSet = const_cast<UAMBCombatAttributeSet*>(RegisteredAttributeSet);
	}

	if (CombatAttributeSet)
	{
		HealthChangedDelegateHandle = CombatAttributeSet->OnHealthChanged().AddUObject(this, &AAMBGASCharacterBase::HandleHealthChanged);
	}

	bIsDead = GetHealth() <= 0.0f;
}

void AAMBGASCharacterBase::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (CombatAttributeSet && HealthChangedDelegateHandle.IsValid())
	{
		CombatAttributeSet->OnHealthChanged().Remove(HealthChangedDelegateHandle);
		HealthChangedDelegateHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void AAMBGASCharacterBase::InitializeAbilityActorInfo()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AAMBGASCharacterBase::HandleHealthChanged(float OldHealth, float NewHealth, AActor* InstigatorActor)
{
	if (GEngine)
	{
		const uint64 MessageKey = static_cast<uint64>(GetUniqueID());
		const FString DebugMessage = FString::Printf(TEXT("%s HP: %.0f / %.0f"), *GetNameSafe(this), NewHealth, GetMaxHealth());
		GEngine->AddOnScreenDebugMessage(MessageKey, 2.0f, NewHealth > 0.0f ? FColor::Yellow : FColor::Red, DebugMessage);
	}

	if (NewHealth < OldHealth)
	{
		const float Damage = OldHealth - NewHealth;
		LastDamageCauser = InstigatorActor;
		BP_OnGASDamaged(Damage, NewHealth, LastDamageLocation, LastDamageImpulse.GetSafeNormal(), InstigatorActor);
	}

	if (!bIsDead && NewHealth <= 0.0f)
	{
		bIsDead = true;
		LastDamageCauser = InstigatorActor;
		HandleDeath();
	}
	else if (NewHealth > 0.0f)
	{
		bIsDead = false;
	}
}

void AAMBGASCharacterBase::HandleOutOfHealth(AActor* InstigatorActor)
{
	UE_LOG(LogMudAndBlood, Log,
		TEXT("%s HandleOutOfHealth Instigator=%s"),
		*GetNameSafe(this),
		*GetNameSafe(InstigatorActor));
	BP_OnGASDied(InstigatorActor);
}

bool AAMBGASCharacterBase::ApplyHealthDeltaToTarget(
	UAbilitySystemComponent* TargetAbilitySystem,
	float HealthDelta,
	AActor* InstigatorActor,
	UObject* SourceObject)
{
	if (!AbilitySystemComponent || !TargetAbilitySystem || FMath::IsNearlyZero(HealthDelta))
	{
		return false;
	}

	TSubclassOf<UGameplayEffect> EffectClass = DamageGameplayEffectClass;
	if (!EffectClass)
	{
		EffectClass = UAMBGameplayEffect_Damage::StaticClass();
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddInstigator(InstigatorActor ? InstigatorActor : this, this);
	EffectContext.AddSourceObject(SourceObject ? SourceObject : this);

	FGameplayEffectSpecHandle EffectSpec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);
	if (!EffectSpec.IsValid())
	{
		return false;
	}

	EffectSpec.Data->SetSetByCallerMagnitude(TAG_Data_Damage, HealthDelta);
	UE_LOG(LogMudAndBlood, Log,
		TEXT("%s ApplyHealthDeltaToTarget Source=%s TargetASC=%s Delta=%f SetByCaller=%f"),
		*GetNameSafe(this),
		*GetNameSafe(InstigatorActor),
		*GetNameSafe(TargetAbilitySystem->GetOwner()),
		HealthDelta,
		EffectSpec.Data->GetSetByCallerMagnitude(TAG_Data_Damage, false, 0.0f));
	AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*EffectSpec.Data.Get(), TargetAbilitySystem);
	return true;
}

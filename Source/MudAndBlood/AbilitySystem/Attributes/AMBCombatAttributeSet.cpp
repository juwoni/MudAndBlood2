#include "AMBCombatAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "MudAndBlood.h"
#include "Net/UnrealNetwork.h"

UAMBCombatAttributeSet::UAMBCombatAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitAttackPower(1.0f);
	InitAttackSpeed(1.0f);
	InitStamina(100.0f);
	InitDrawStrength(1.0f);
}

void UAMBCombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAMBCombatAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAMBCombatAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAMBCombatAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAMBCombatAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAMBCombatAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAMBCombatAttributeSet, DrawStrength, COND_None, REPNOTIFY_Always);
}

void UAMBCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UAMBCombatAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAMBCombatAttributeSet, Health, OldHealth);
}

void UAMBCombatAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAMBCombatAttributeSet, MaxHealth, OldMaxHealth);
}

void UAMBCombatAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAMBCombatAttributeSet, AttackPower, OldAttackPower);
}

void UAMBCombatAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAMBCombatAttributeSet, AttackSpeed, OldAttackSpeed);
}

void UAMBCombatAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAMBCombatAttributeSet, Stamina, OldStamina);
}

void UAMBCombatAttributeSet::OnRep_DrawStrength(const FGameplayAttributeData& OldDrawStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAMBCombatAttributeSet, DrawStrength, OldDrawStrength);
}

void UAMBCombatAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float NewHealth = FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth());
		const float PreviousHealth = FMath::Clamp(NewHealth - Data.EvaluatedData.Magnitude, 0.0f, GetMaxHealth());
		SetHealth(NewHealth);
		AActor* InstigatorActor = Data.EffectSpec.GetContext().GetOriginalInstigator();
		if (!InstigatorActor)
		{
			InstigatorActor = Data.EffectSpec.GetContext().GetInstigator();
		}
		UE_LOG(LogMudAndBlood, Log,
			TEXT("%s Health execute ASC=%p Magnitude=%f OldCurrent=%f NewCurrent=%f Max=%f Instigator=%s"),
			*GetNameSafe(GetOwningActor()),
			GetOwningAbilitySystemComponent(),
			Data.EvaluatedData.Magnitude,
			PreviousHealth,
			NewHealth,
			GetMaxHealth(),
			*GetNameSafe(InstigatorActor));
		if (!FMath::IsNearlyEqual(PreviousHealth, NewHealth))
		{
			HealthChangedDelegate.Broadcast(PreviousHealth, NewHealth, InstigatorActor);
		}
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetMaxHealth(FMath::Max(GetMaxHealth(), 0.0f));
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
}

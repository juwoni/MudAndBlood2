#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "AMBCombatAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_MULTICAST_DELEGATE_ThreeParams(FAMBHealthChangedSignature, float, float, AActor*);

UCLASS()
class MUDANDBLOOD_API UAMBCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAMBCombatAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	FAMBHealthChangedSignature& OnHealthChanged() { return HealthChangedDelegate; }

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="Combat Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAMBCombatAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, Category="Combat Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAMBCombatAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_AttackPower, Category="Combat Attributes")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UAMBCombatAttributeSet, AttackPower)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_AttackSpeed, Category="Combat Attributes")
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(UAMBCombatAttributeSet, AttackSpeed)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Stamina, Category="Combat Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UAMBCombatAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_DrawStrength, Category="Combat Attributes")
	FGameplayAttributeData DrawStrength;
	ATTRIBUTE_ACCESSORS(UAMBCombatAttributeSet, DrawStrength)

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed);

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	UFUNCTION()
	void OnRep_DrawStrength(const FGameplayAttributeData& OldDrawStrength);

private:
	FAMBHealthChangedSignature HealthChangedDelegate;
};

#undef ATTRIBUTE_ACCESSORS

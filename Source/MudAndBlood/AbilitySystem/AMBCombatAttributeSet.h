#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AMBCombatAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class MUDANDBLOOD_API UAMBCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAMBCombatAttributeSet();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Attributes")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UAMBCombatAttributeSet, AttackPower)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Attributes")
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(UAMBCombatAttributeSet, AttackSpeed)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UAMBCombatAttributeSet, Stamina)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Attributes")
	FGameplayAttributeData DrawStrength;
	ATTRIBUTE_ACCESSORS(UAMBCombatAttributeSet, DrawStrength)
};

#undef ATTRIBUTE_ACCESSORS

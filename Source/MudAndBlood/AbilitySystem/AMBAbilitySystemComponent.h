#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AMBAbilitySystemComponent.generated.h"

UCLASS()
class MUDANDBLOOD_API UAMBAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Abilities")
	bool HasAbilityWithInputTag(FGameplayTag InputTag) const;

	UFUNCTION(BlueprintCallable, Category="Abilities")
	bool TryActivateAbilitiesByInputTag(FGameplayTag InputTag, bool bAllowRemoteActivation = true);
};

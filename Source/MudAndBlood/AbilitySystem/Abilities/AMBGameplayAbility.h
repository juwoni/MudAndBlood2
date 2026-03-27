#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "AMBGameplayAbility.generated.h"

UCLASS(Abstract, Blueprintable)
class MUDANDBLOOD_API UAMBGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAMBGameplayAbility();

	bool DoesAbilityMatchInputTag(FGameplayTag InputTag) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FGameplayTag AbilityInputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FGameplayTagContainer RequiredCombatStyleTags;

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;
};

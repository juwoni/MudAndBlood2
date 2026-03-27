#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AMBCombatStyleData.generated.h"

class UAnimMontage;
class UGameplayAbility;

UENUM(BlueprintType)
enum class EAMBCombatStyleType : uint8
{
	Unarmed,
	Sword,
	Bow
};

USTRUCT(BlueprintType)
struct FAMBCombatAbilityGrant
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat", meta = (ClampMin = 1))
	int32 AbilityLevel = 1;
};

UCLASS(BlueprintType)
class MUDANDBLOOD_API UAMBCombatStyleData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	EAMBCombatStyleType CombatStyleType = EAMBCombatStyleType::Unarmed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FGameplayTag CombatStyleTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	TArray<FAMBCombatAbilityGrant> GrantedAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Combo")
	TObjectPtr<UAnimMontage> ComboAttackMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Combo")
	TArray<FName> ComboSectionNames;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Combo", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float ComboInputCacheTimeTolerance = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Charged")
	TObjectPtr<UAnimMontage> ChargedAttackMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Charged")
	FName ChargeLoopSection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Charged")
	FName ChargeAttackSection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Timing", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float AttackInputCacheTimeTolerance = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Trace", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float MeleeTraceDistance = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Trace", meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
	float MeleeTraceRadius = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Trace", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float DangerTraceDistance = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Trace", meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
	float DangerTraceRadius = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Damage", meta = (ClampMin = 0, ClampMax = 100))
	float MeleeDamage = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MeleeKnockbackImpulse = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MeleeLaunchImpulse = 300.0f;
};

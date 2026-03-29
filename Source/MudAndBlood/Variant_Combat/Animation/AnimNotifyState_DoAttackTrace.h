// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_DoAttackTrace.generated.h"

/**
 * AnimNotifyState that keeps a melee trace window open for the duration of the notify.
 */
UCLASS()
class MUDANDBLOOD_API UAnimNotifyState_DoAttackTrace : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	/** Start socket or bone for the attack trace. Usually weapon base/guard or hand socket. */
	UPROPERTY(EditAnywhere, Category="Attack")
	FName AttackBoneName;

	/** End socket or bone for the attack trace. Usually weapon tip. If unset, the trace falls back to forward sweep. */
	UPROPERTY(EditAnywhere, Category="Attack")
	FName AttackEndBoneName;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};

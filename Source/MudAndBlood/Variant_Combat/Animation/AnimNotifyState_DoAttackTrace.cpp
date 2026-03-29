// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimNotifyState_DoAttackTrace.h"

#include "CombatAttacker.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_DoAttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (ICombatAttacker* AttackerInterface = Cast<ICombatAttacker>(MeshComp ? MeshComp->GetOwner() : nullptr))
	{
		AttackerInterface->BeginAttackTraceWindow(AttackBoneName, AttackEndBoneName);
	}
}

void UAnimNotifyState_DoAttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (ICombatAttacker* AttackerInterface = Cast<ICombatAttacker>(MeshComp ? MeshComp->GetOwner() : nullptr))
	{
		AttackerInterface->TickAttackTraceWindow(AttackBoneName, AttackEndBoneName);
	}
}

void UAnimNotifyState_DoAttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ICombatAttacker* AttackerInterface = Cast<ICombatAttacker>(MeshComp ? MeshComp->GetOwner() : nullptr))
	{
		AttackerInterface->EndAttackTraceWindow();
	}
}

FString UAnimNotifyState_DoAttackTrace::GetNotifyName_Implementation() const
{
	return FString("Do Attack Trace State");
}

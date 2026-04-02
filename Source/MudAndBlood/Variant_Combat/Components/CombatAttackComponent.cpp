// Copyright Epic Games, Inc. All Rights Reserved.

#include "Variant_Combat/Components/CombatAttackComponent.h"

#include "AMBCharacter.h"
#include "Variant_Combat/Data/AMBCombatStyleData.h"
#include "CombatDamageable.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UCombatAttackComponent::UCombatAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	OnAttackMontageEnded.BindUObject(this, &UCombatAttackComponent::AttackMontageEnded);
}

void UCombatAttackComponent::SetWeaponTrace(bool isTracing)
{
	bWeaponTrace = isTracing;
}

void UCombatAttackComponent::SetMeleeTraceSettings(float TraceDistance, float TraceRadius, float Damage,
                                                   float KnockbackImpulse, float LaunchImpulse)
{
	MeleeTraceDistance = TraceDistance;
	MeleeTraceRadius = TraceRadius;
	MeleeDamage = Damage;
	MeleeKnockbackImpulse = KnockbackImpulse;
	MeleeLaunchImpulse = LaunchImpulse;
}

void UCombatAttackComponent::DoComboAttackStart()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (bIsAttacking)
	{
		CachedAttackInputTime = World->GetTimeSeconds();
		return;
	}

	ComboAttack();
}

void UCombatAttackComponent::DoComboAttackEnd()
{
	// Stub kept for parity with ACombatCharacter's original public API.
}

void UCombatAttackComponent::ApplyWeaponDamage(AActor* HitActor, const FVector& ImpactPoint)
{
	if (!GetWorld() || !IsValid(HitActor))
	{
		return;
	}

	DrawDebugString(
		GetWorld(),
		ImpactPoint,
		TEXT("Hello"),
		GetOwner(),
		FColor::White,
		2.0f, // 지속 시간
		true
	);
}

bool UCombatAttackComponent::AttackBoxTrace()
{
	AAMBCharacter* CharacterOwner = Cast<AAMBCharacter>(GetOwner());
	if (!CharacterOwner)
	{
		return false;
	}

	UStaticMeshComponent* EquippedWeaponMesh = CharacterOwner->GetEquippedWeaponMesh();
	if (!EquippedWeaponMesh)
	{
		return false;
	}

	FVector localCurrentTopTrace = EquippedWeaponMesh->GetSocketLocation(TEXT("TopTrace"));
	FVector localCurrentBottomTrace = EquippedWeaponMesh->GetSocketLocation(TEXT("BottomTrace"));

	const FVector BoxExtent(FVector::Distance(localCurrentTopTrace, localCurrentBottomTrace) * 0.5f, 10.0f, 10.0f);

	const FRotator TraceRotation =
		UKismetMathLibrary::FindLookAtRotation(localCurrentTopTrace, localCurrentBottomTrace);


	TArray<FHitResult> OutHits;

	const bool bHit = UKismetSystemLibrary::BoxTraceMulti(
		this,
		PreTopSocket,
		localCurrentTopTrace,
		BoxExtent,
		TraceRotation,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		// {
		// 	UEngineTypes::ConvertToObjectType(ECC_Visibility),
		// 	UEngineTypes::ConvertToObjectType(ECC_Pawn),
		// 	
		// 	
		// },
		false,
		{
			CharacterOwner
		},
		EDrawDebugTrace::ForDuration,
		// bDrawWeaponDamageTraceDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		OutHits,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		WeaponDamageTraceDebugDuration);

	PreTopSocket = localCurrentTopTrace;
	PreBottomSocket = localCurrentBottomTrace;

	if (OutHits.Num() >= 0)
	{
	}


	return bHit;
}

bool UCombatAttackComponent::AttackSphereTrace(FName TraceStartBone, FName TraceEndBone,
                                               FHitResult& OutHitResult)
{
	OutHitResult = FHitResult();

	ACharacter* CharacterOwner = GetCharacterOwner();
	if (!CharacterOwner || !CharacterOwner->GetMesh())
	{
		return false;
	}

	TArray<FHitResult> OutHits;

	const FVector TraceStart = CharacterOwner->GetMesh()->GetSocketLocation(TraceStartBone);
	const FVector TraceEnd = TraceEndBone.IsNone()
		                         ? TraceStart + (CharacterOwner->GetActorForwardVector() * MeleeTraceDistance)
		                         : CharacterOwner->GetMesh()->GetSocketLocation(TraceEndBone);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(CharacterOwner);

	const bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		this,
		TraceStart,
		TraceEnd,
		MeleeTraceRadius,
		ObjectTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		// EDrawDebugTrace::None,
		// bDrawWeaponDamageTraceDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		OutHits,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		WeaponDamageTraceDebugDuration);

	if (!bHit)
	{
		return false;
	}

	bool bFoundDamageableHit = false;

	for (const FHitResult& CurrentHit : OutHits)
	{
		AActor* CurrentHitActor = CurrentHit.GetActor();
		if (!IsValid(CurrentHitActor))
		{
			continue;
		}

		ICombatDamageable* Damageable = Cast<ICombatDamageable>(CurrentHitActor);
		if (!Damageable)
		{
			continue;
		}

		if (!bFoundDamageableHit)
		{
			OutHitResult = CurrentHit;
			bFoundDamageableHit = true;
		}

		// if (!HitActor)
		// {
		// 	HitActor = CurrentHitActor;
		// 	ImpactPoint = CurrentHit.ImpactPoint;
		// }

		// const FVector Impulse = (CurrentHit.ImpactNormal * -MeleeKnockbackImpulse) + (FVector::UpVector * MeleeLaunchImpulse);
		//
		// if (AAMBCharacter* PlayerCharacter = Cast<AAMBCharacter>(CharacterOwner))
		// {
		// 	if (PlayerCharacter->ApplyCurrentWeaponDamageToTarget(CurrentHitActor, CurrentHit.ImpactPoint, Impulse))
		// 	{
		// 		OnDamageDealt.Broadcast(PlayerCharacter->GetCurrentWeaponDamage(), CurrentHit.ImpactPoint);
		// 	}
		// }
		// else
		// {
		// 	Damageable->ApplyDamage(MeleeDamage, CharacterOwner, CurrentHit.ImpactPoint, Impulse);
		// }
	}

	return bFoundDamageableHit;
}

void UCombatAttackComponent::DoAttackTrace_Implementation(FName TraceStartBone, FName TraceEndBone)
{
	FHitResult HitResult;
	AttackSphereTrace(TraceStartBone, TraceEndBone, HitResult);
}

void UCombatAttackComponent::BeginAttackTraceWindow_Implementation(FName TraceStartBone, FName TraceEndBone)
{
	static_cast<void>(TraceStartBone);
	static_cast<void>(TraceEndBone);
}

void UCombatAttackComponent::TickAttackTraceWindow_Implementation(FName TraceStartBone, FName TraceEndBone)
{
	static_cast<void>(TraceStartBone);
	static_cast<void>(TraceEndBone);
}

void UCombatAttackComponent::EndAttackTraceWindow_Implementation()
{
}

void UCombatAttackComponent::CheckCombo()
{
	UWorld* World = GetWorld();
	if (!World || !bIsAttacking)
	{
		return;
	}

	if (World->GetTimeSeconds() - CachedAttackInputTime > ComboInputCacheTimeTolerance)
	{
		return;
	}

	CachedAttackInputTime = 0.0f;
	++ComboCount;

	if (ComboCount >= ComboSectionNames.Num())
	{
		return;
	}

	// NotifyEnemiesOfIncomingAttack();

	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		AnimInstance->Montage_JumpToSection(ComboSectionNames[ComboCount], ComboAttackMontage);
	}
}

void UCombatAttackComponent::NotifyEnemiesOfIncomingAttack_Implementation()
{
}

void UCombatAttackComponent::ApplyCombatStyleData(UAMBCombatStyleData* CombatStyleData)
{
	if (!CombatStyleData)
	{
		return;
	}

	CachedAttackInputTime = 0.0f;
	bIsAttacking = false;
	ComboCount = 0;

	AttackInputCacheTimeTolerance = CombatStyleData->AttackInputCacheTimeTolerance;
	MeleeTraceDistance = CombatStyleData->MeleeTraceDistance;
	MeleeTraceRadius = CombatStyleData->MeleeTraceRadius;
	DangerTraceDistance = CombatStyleData->DangerTraceDistance;
	DangerTraceRadius = CombatStyleData->DangerTraceRadius;
	MeleeDamage = CombatStyleData->MeleeDamage;
	MeleeKnockbackImpulse = CombatStyleData->MeleeKnockbackImpulse;
	MeleeLaunchImpulse = CombatStyleData->MeleeLaunchImpulse;
	ComboAttackMontage = CombatStyleData->ComboAttackMontage;
	ComboSectionNames = CombatStyleData->ComboSectionNames;
	ComboInputCacheTimeTolerance = CombatStyleData->ComboInputCacheTimeTolerance;
	ChargedAttackMontage = CombatStyleData->ChargedAttackMontage;
	ChargeLoopSection = CombatStyleData->ChargeLoopSection;
	ChargeAttackSection = CombatStyleData->ChargeAttackSection;
}

void UCombatAttackComponent::ComboAttack()
{
	bIsAttacking = true;
	ComboCount = 0;

	// NotifyEnemiesOfIncomingAttack();

	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(ComboAttackMontage, 1.0f,
		                                                       EMontagePlayReturnType::MontageLength, 0.0f, true);
		if (MontageLength > 0.0f)
		{
			AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, ComboAttackMontage);
		}
	}
}

bool UCombatAttackComponent::PlayChargedAttackMontage()
{
	if (bIsAttacking || !ChargedAttackMontage)
	{
		return false;
	}

	bIsAttacking = true;

	NotifyEnemiesOfIncomingAttack();

	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(ChargedAttackMontage, 1.0f,
		                                                       EMontagePlayReturnType::MontageLength, 0.0f, true);
		if (MontageLength > 0.0f)
		{
			AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, ChargedAttackMontage);
			return true;
		}
	}

	bIsAttacking = false;
	return false;
}

void UCombatAttackComponent::AdvanceChargedAttack(bool bShouldLoopCharge)
{
	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		AnimInstance->Montage_JumpToSection(bShouldLoopCharge ? ChargeLoopSection : ChargeAttackSection,
		                                    ChargedAttackMontage);
	}
}

void UCombatAttackComponent::StopChargedAttackMontage(float BlendOutTime)
{
	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		AnimInstance->Montage_Stop(BlendOutTime, ChargedAttackMontage);
	}
}

void UCombatAttackComponent::AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		bIsAttacking = false;
		return;
	}

	bIsAttacking = false;
	OnAttackMontageFinished.Broadcast(Montage, bInterrupted);

	if (World->GetTimeSeconds() - CachedAttackInputTime > AttackInputCacheTimeTolerance)
	{
		return;
	}

	ComboAttack();
}

ACharacter* UCombatAttackComponent::GetCharacterOwner() const
{
	return Cast<ACharacter>(GetOwner());
}

UAnimInstance* UCombatAttackComponent::GetOwnerAnimInstance() const
{
	ACharacter* CharacterOwner = GetCharacterOwner();
	if (!CharacterOwner || !CharacterOwner->GetMesh())
	{
		return nullptr;
	}

	return CharacterOwner->GetMesh()->GetAnimInstance();
}

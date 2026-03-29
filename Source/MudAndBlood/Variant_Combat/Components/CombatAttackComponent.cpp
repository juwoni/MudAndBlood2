// Copyright Epic Games, Inc. All Rights Reserved.

#include "Variant_Combat/Components/CombatAttackComponent.h"

#include "AMBCharacter.h"
#include "Variant_Combat/Data/AMBCombatStyleData.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CombatDamageable.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

const FName UCombatAttackComponent::DefaultWeaponAttackTraceStartSocketName(TEXT("AttackStart"));
const FName UCombatAttackComponent::DefaultWeaponAttackTraceEndSocketName(TEXT("AttackEnd"));

UCombatAttackComponent::UCombatAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	OnAttackMontageEnded.BindUObject(this, &UCombatAttackComponent::AttackMontageEnded);
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

void UCombatAttackComponent::DoAttackTrace(FName TraceStartBone, FName TraceEndBone)
{
	ACharacter* CharacterOwner = GetCharacterOwner();
	if (!CharacterOwner)
	{
		return;
	}

	FVector TraceStart = FVector::ZeroVector;
	FVector TraceEnd = FVector::ZeroVector;
	if (!TryResolveAttackTracePoints(CharacterOwner, TraceStartBone, TraceEndBone, TraceStart, TraceEnd))
	{
		return;
	}

	TSet<TWeakObjectPtr<AActor>> DamagedActors;
	PerformAttackTraceSweep(CharacterOwner, TraceStart, TraceEnd, DamagedActors);
}

void UCombatAttackComponent::BeginAttackTraceWindow(FName TraceStartBone, FName TraceEndBone)
{
	ACharacter* CharacterOwner = GetCharacterOwner();
	if (!CharacterOwner)
	{
		return;
	}

	ResetAttackTraceWindowState();
	bIsAttackTraceWindowActive = true;

	FVector TraceStart = FVector::ZeroVector;
	FVector TraceEnd = FVector::ZeroVector;
	if (!TryResolveAttackTracePoints(CharacterOwner, TraceStartBone, TraceEndBone, TraceStart, TraceEnd))
	{
		ResetAttackTraceWindowState();
		return;
	}

	PreviousAttackTraceStart = TraceStart;
	PreviousAttackTraceEnd = TraceEnd;
	bHasPreviousAttackTracePoints = true;

	PerformAttackTraceSweep(CharacterOwner, TraceStart, TraceEnd, DamagedActorsInTraceWindow);
}

void UCombatAttackComponent::TickAttackTraceWindow(FName TraceStartBone, FName TraceEndBone)
{
	if (!bIsAttackTraceWindowActive)
	{
		return;
	}

	ACharacter* CharacterOwner = GetCharacterOwner();
	if (!CharacterOwner)
	{
		ResetAttackTraceWindowState();
		return;
	}

	FVector CurrentTraceStart = FVector::ZeroVector;
	FVector CurrentTraceEnd = FVector::ZeroVector;
	if (!TryResolveAttackTracePoints(CharacterOwner, TraceStartBone, TraceEndBone, CurrentTraceStart, CurrentTraceEnd))
	{
		return;
	}

	if (!bHasPreviousAttackTracePoints)
	{
		PreviousAttackTraceStart = CurrentTraceStart;
		PreviousAttackTraceEnd = CurrentTraceEnd;
		bHasPreviousAttackTracePoints = true;
	}

	PerformAttackTraceSweep(CharacterOwner, CurrentTraceStart, CurrentTraceEnd, DamagedActorsInTraceWindow);

	if (!PreviousAttackTraceStart.Equals(CurrentTraceStart))
	{
		PerformAttackTraceSweep(CharacterOwner, PreviousAttackTraceStart, CurrentTraceStart, DamagedActorsInTraceWindow);
	}

	if (!PreviousAttackTraceEnd.Equals(CurrentTraceEnd))
	{
		PerformAttackTraceSweep(CharacterOwner, PreviousAttackTraceEnd, CurrentTraceEnd, DamagedActorsInTraceWindow);
	}

	PreviousAttackTraceStart = CurrentTraceStart;
	PreviousAttackTraceEnd = CurrentTraceEnd;
}

void UCombatAttackComponent::EndAttackTraceWindow()
{
	ResetAttackTraceWindowState();
}

bool UCombatAttackComponent::TryResolveAttackTraceLocation(ACharacter* CharacterOwner, FName SocketName, FVector& OutLocation) const
{
	if (!CharacterOwner || SocketName.IsNone())
	{
		return false;
	}

	if (const AAMBCharacter* CombatCharacterOwner = Cast<AAMBCharacter>(CharacterOwner))
	{
		if (UStaticMeshComponent* EquippedWeaponMesh = CombatCharacterOwner->GetEquippedItemMeshComponent())
		{
			if (EquippedWeaponMesh->IsRegistered() && EquippedWeaponMesh->DoesSocketExist(SocketName))
			{
				OutLocation = EquippedWeaponMesh->GetSocketLocation(SocketName);
				return true;
			}
		}
	}

	return false;
}

bool UCombatAttackComponent::TryResolveAttackTracePoints(
	ACharacter* CharacterOwner,
	FName TraceStartBone,
	FName TraceEndBone,
	FVector& OutTraceStart,
	FVector& OutTraceEnd) const
{
	static_cast<void>(TraceStartBone);
	static_cast<void>(TraceEndBone);

	if (!TryResolveAttackTraceLocation(CharacterOwner, DefaultWeaponAttackTraceStartSocketName, OutTraceStart))
	{
		return false;
	}

	if (!TryResolveAttackTraceLocation(CharacterOwner, DefaultWeaponAttackTraceEndSocketName, OutTraceEnd))
	{
		return false;
	}

	return true;
}

void UCombatAttackComponent::PerformAttackTraceSweep(
	ACharacter* CharacterOwner,
	const FVector& TraceStart,
	const FVector& TraceEnd,
	TSet<TWeakObjectPtr<AActor>>& AlreadyHitActors)
{
	UWorld* World = CharacterOwner ? CharacterOwner->GetWorld() : nullptr;
	if (!CharacterOwner || !World)
	{
		return;
	}

	FHitResult OutHit;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(CharacterOwner);

	const FVector TraceDirection = (TraceEnd - TraceStart).GetSafeNormal();
	const FRotator TraceOrientation = TraceDirection.IsNearlyZero()
		? CharacterOwner->GetActorForwardVector().Rotation()
		: TraceDirection.Rotation();
	const FVector BoxHalfSize(MeleeTraceRadius, MeleeTraceRadius * 0.35f, MeleeTraceRadius * 0.35f);

	const bool bHit = UKismetSystemLibrary::BoxTraceSingle(
		World,
		TraceStart,
		TraceEnd,
		BoxHalfSize,
		TraceOrientation,
		GetAttackTraceChannel(),
		false,
		ActorsToIgnore,
		GetAttackTraceDrawDebugType(),
		OutHit,
		false,
		FLinearColor(0.0f, 1.0f, 1.0f, 1.0f),
		FLinearColor::Red,
		GetAttackTraceDebugLifetime(World)
	);

	if (!bHit)
	{
		return;
	}

	AActor* HitActor = OutHit.GetActor();
	TWeakObjectPtr<AActor> HitActorPtr(HitActor);
	if (!HitActor || AlreadyHitActors.Contains(HitActorPtr))
	{
		return;
	}

	ICombatDamageable* Damageable = Cast<ICombatDamageable>(HitActor);
	if (!Damageable)
	{
		return;
	}

	AlreadyHitActors.Add(HitActorPtr);

	const FVector Impulse = (OutHit.ImpactNormal * -MeleeKnockbackImpulse) + (FVector::UpVector * MeleeLaunchImpulse);

	Damageable->ApplyDamage(MeleeDamage, CharacterOwner, OutHit.ImpactPoint, Impulse);
	OnDamageDealt.Broadcast(MeleeDamage, OutHit.ImpactPoint);
}

ETraceTypeQuery UCombatAttackComponent::GetAttackTraceChannel() const
{
	return UEngineTypes::ConvertToTraceType(ECC_Visibility);
}

EDrawDebugTrace::Type UCombatAttackComponent::GetAttackTraceDrawDebugType() const
{
	if (!bDrawWeaponDamageTraceDebug)
	{
		return EDrawDebugTrace::None;
	}

	return bIsAttackTraceWindowActive ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::ForDuration;
}

float UCombatAttackComponent::GetAttackTraceDebugLifetime(const UWorld* World) const
{
	if (!bIsAttackTraceWindowActive)
	{
		return WeaponDamageTraceDebugDuration;
	}

	if (!World)
	{
		return 0.0f;
	}

	// When the trace is driven by an AnimNotifyState, keep debug geometry alive for roughly one frame
	// so it visually tracks the montage instead of lingering after the swing has ended.
	return FMath::Max(World->GetDeltaSeconds(), KINDA_SMALL_NUMBER);
}

void UCombatAttackComponent::ResetAttackTraceWindowState()
{
	bIsAttackTraceWindowActive = false;
	bHasPreviousAttackTracePoints = false;
	PreviousAttackTraceStart = FVector::ZeroVector;
	PreviousAttackTraceEnd = FVector::ZeroVector;
	DamagedActorsInTraceWindow.Reset();
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

void UCombatAttackComponent::NotifyEnemiesOfIncomingAttack()
{
	ACharacter* CharacterOwner = GetCharacterOwner();
	UWorld* World = GetWorld();

	if (!CharacterOwner || !World)
	{
		return;
	}

	TArray<FHitResult> OutHits;

	const FVector TraceStart = CharacterOwner->GetActorLocation();
	const FVector TraceEnd = TraceStart + (CharacterOwner->GetActorForwardVector() * DangerTraceDistance);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(DangerTraceRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner);

	const bool bHit = World->SweepMultiByObjectType(OutHits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, CollisionShape, QueryParams);

	if (!bHit)
	{
		return;
	}

	for (const FHitResult& CurrentHit : OutHits)
	{
		ICombatDamageable* Damageable = Cast<ICombatDamageable>(CurrentHit.GetActor());
		if (!Damageable)
		{
			continue;
		}

		Damageable->NotifyDanger(CharacterOwner->GetActorLocation(), CharacterOwner);
	}
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
	ResetAttackTraceWindowState();

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
		const float MontageLength = AnimInstance->Montage_Play(ComboAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
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
		const float MontageLength = AnimInstance->Montage_Play(ChargedAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
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
		AnimInstance->Montage_JumpToSection(bShouldLoopCharge ? ChargeLoopSection : ChargeAttackSection, ChargedAttackMontage);
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

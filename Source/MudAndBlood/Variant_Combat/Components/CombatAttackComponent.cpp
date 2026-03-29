// Copyright Epic Games, Inc. All Rights Reserved.

#include "Variant_Combat/Components/CombatAttackComponent.h"

#include "AMBCharacter.h"
#include "Variant_Combat/Data/AMBCombatStyleData.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CombatDamageable.h"
#include "DrawDebugHelpers.h"

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

void UCombatAttackComponent::DoChargedAttackStart()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bIsChargingAttack = true;

	if (bIsAttacking)
	{
		CachedAttackInputTime = World->GetTimeSeconds();
		return;
	}

	ChargedAttack();
}

void UCombatAttackComponent::DoChargedAttackEnd()
{
	bIsChargingAttack = false;

	if (bHasLoopedChargedAttack)
	{
		CheckChargedAttack();
	}
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

	TArray<FHitResult> OutHits;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(MeleeTraceRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner);

	const bool bHit = World->SweepMultiByObjectType(OutHits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, CollisionShape, QueryParams);

	if (bDrawWeaponDamageTraceDebug)
	{
		const FColor TraceColor = bHit ? FColor::Red : FColor::Cyan;
		DrawDebugSphere(World, TraceStart, MeleeTraceRadius, 16, TraceColor, false, WeaponDamageTraceDebugDuration);
		DrawDebugSphere(World, TraceEnd, MeleeTraceRadius, 16, TraceColor, false, WeaponDamageTraceDebugDuration);
		DrawDebugLine(World, TraceStart, TraceEnd, TraceColor, false, WeaponDamageTraceDebugDuration, 0, 2.0f);

		for (const FHitResult& CurrentHit : OutHits)
		{
			if (!CurrentHit.GetActor())
			{
				continue;
			}

			DrawDebugPoint(World, CurrentHit.ImpactPoint, 16.0f, FColor::Yellow, false, WeaponDamageTraceDebugDuration);
		}
	}

	if (!bHit)
	{
		return;
	}

	for (const FHitResult& CurrentHit : OutHits)
	{
		AActor* HitActor = CurrentHit.GetActor();
		TWeakObjectPtr<AActor> HitActorPtr(HitActor);
		if (!HitActor || AlreadyHitActors.Contains(HitActorPtr))
		{
			continue;
		}

		ICombatDamageable* Damageable = Cast<ICombatDamageable>(HitActor);
		if (!Damageable)
		{
			continue;
		}

		AlreadyHitActors.Add(HitActorPtr);

		const FVector Impulse = (CurrentHit.ImpactNormal * -MeleeKnockbackImpulse) + (FVector::UpVector * MeleeLaunchImpulse);

		Damageable->ApplyDamage(MeleeDamage, CharacterOwner, CurrentHit.ImpactPoint, Impulse);
		OnDamageDealt.Broadcast(MeleeDamage, CurrentHit.ImpactPoint);
	}
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
	if (!World || !bIsAttacking || bIsChargingAttack)
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

	NotifyEnemiesOfIncomingAttack();

	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		AnimInstance->Montage_JumpToSection(ComboSectionNames[ComboCount], ComboAttackMontage);
	}
}

void UCombatAttackComponent::CheckChargedAttack()
{
	bHasLoopedChargedAttack = true;

	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		AnimInstance->Montage_JumpToSection(bIsChargingAttack ? ChargeLoopSection : ChargeAttackSection, ChargedAttackMontage);
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

	if (bDrawDangerTraceDebug)
	{
		const FVector TraceVector = TraceEnd - TraceStart;
		const FVector TraceCenter = TraceStart + (TraceVector * 0.5f);
		const float HalfHeight = (TraceVector.Size() * 0.5f) + DangerTraceRadius;
		const FQuat TraceRotation = TraceVector.IsNearlyZero()
			? FQuat::Identity
			: FQuat::FindBetweenNormals(FVector::UpVector, TraceVector.GetSafeNormal());
		const FColor TraceColor = bHit ? FColor::Orange : FColor::Green;

		DrawDebugCapsule(World, TraceCenter, HalfHeight, DangerTraceRadius, TraceRotation, TraceColor, false, DangerTraceDebugDuration);
		DrawDebugSphere(World, TraceStart, DangerTraceRadius, 16, TraceColor, false, DangerTraceDebugDuration);
		DrawDebugSphere(World, TraceEnd, DangerTraceRadius, 16, TraceColor, false, DangerTraceDebugDuration);

		for (const FHitResult& CurrentHit : OutHits)
		{
			if (!CurrentHit.GetActor())
			{
				continue;
			}

			DrawDebugPoint(World, CurrentHit.ImpactPoint, 16.0f, FColor::Red, false, DangerTraceDebugDuration);
		}
	}

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
	bIsChargingAttack = false;
	bHasLoopedChargedAttack = false;
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

	NotifyEnemiesOfIncomingAttack();

	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(ComboAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		if (MontageLength > 0.0f)
		{
			AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, ComboAttackMontage);
		}
	}
}

void UCombatAttackComponent::ChargedAttack()
{
	bIsAttacking = true;
	bHasLoopedChargedAttack = false;

	NotifyEnemiesOfIncomingAttack();

	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(ChargedAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		if (MontageLength > 0.0f)
		{
			AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, ChargedAttackMontage);
		}
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

	if (World->GetTimeSeconds() - CachedAttackInputTime > AttackInputCacheTimeTolerance)
	{
		return;
	}

	if (bIsChargingAttack)
	{
		ChargedAttack();
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

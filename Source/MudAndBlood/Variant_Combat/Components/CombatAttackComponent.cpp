// Copyright Epic Games, Inc. All Rights Reserved.

#include "Variant_Combat/Components/CombatAttackComponent.h"

#include "AMBCharacter.h"
#include "Variant_Combat/Data/AMBCombatStyleData.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CombatDamageable.h"
#include "DrawDebugHelpers.h"
#include "MudAndBlood.h"
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
	PerformAttackTraceSweep(CharacterOwner, TraceStart, TraceEnd, TraceStart, TraceEnd, DamagedActors);
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

	PerformAttackTraceSweep(CharacterOwner, TraceStart, TraceEnd, TraceStart, TraceEnd, DamagedActorsInTraceWindow);
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

	PerformAttackTraceSweep(
		CharacterOwner,
		PreviousAttackTraceStart,
		PreviousAttackTraceEnd,
		CurrentTraceStart,
		CurrentTraceEnd,
		DamagedActorsInTraceWindow);

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

	if (UStaticMeshComponent* EquippedWeaponMesh = GetEquippedWeaponMesh(CharacterOwner))
	{
		const UStaticMesh* EquippedStaticMesh = EquippedWeaponMesh->GetStaticMesh();
		if (EquippedWeaponMesh->IsRegistered() && EquippedWeaponMesh->DoesSocketExist(SocketName))
		{
			OutLocation = EquippedWeaponMesh->GetSocketLocation(SocketName);
			return true;
		}

		UE_LOG(LogMudAndBlood, Warning,
		       TEXT("%s failed to resolve attack socket %s on equipped mesh component. Mesh=%s Registered=%s Hidden=%s"),
		       *GetNameSafe(CharacterOwner),
		       *SocketName.ToString(),
		       *GetNameSafe(EquippedStaticMesh),
		       EquippedWeaponMesh->IsRegistered() ? TEXT("true") : TEXT("false"),
		       EquippedWeaponMesh->bHiddenInGame ? TEXT("true") : TEXT("false"));
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
	const FVector& PreviousTraceStart,
	const FVector& PreviousTraceEnd,
	const FVector& CurrentTraceStart,
	const FVector& CurrentTraceEnd,
	TSet<TWeakObjectPtr<AActor>>& AlreadyHitActors)
{
	UWorld* World = CharacterOwner ? CharacterOwner->GetWorld() : nullptr;
	if (!CharacterOwner || !World)
	{
		return;
	}

	FVector PreviousCenter = FVector::ZeroVector;
	FRotator PreviousOrientation = FRotator::ZeroRotator;
	FVector PreviousHalfSize = FVector::ZeroVector;
	if (!BuildAttackTraceBox(
		CharacterOwner,
		PreviousTraceStart,
		PreviousTraceEnd,
		PreviousCenter,
		PreviousOrientation,
		PreviousHalfSize))
	{
		return;
	}

	FVector CurrentCenter = FVector::ZeroVector;
	FRotator CurrentOrientation = FRotator::ZeroRotator;
	FVector CurrentHalfSize = FVector::ZeroVector;
	if (!BuildAttackTraceBox(
		CharacterOwner,
		CurrentTraceStart,
		CurrentTraceEnd,
		CurrentCenter,
		CurrentOrientation,
		CurrentHalfSize))
	{
		return;
	}

	TArray<FHitResult> OutHits;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(CharacterOwner);

	FVector TraceDirection = PreviousOrientation.Vector() + CurrentOrientation.Vector();
	if (TraceDirection.IsNearlyZero())
	{
		TraceDirection = CurrentOrientation.Vector().IsNearlyZero()
			? (PreviousOrientation.Vector().IsNearlyZero() ? CharacterOwner->GetActorForwardVector() : PreviousOrientation.Vector())
			: CurrentOrientation.Vector();
	}

	const FVector BoxTraceStart = PreviousCenter;
	const FVector BoxTraceEnd = CurrentCenter;
	const FRotator TraceOrientation = TraceDirection.Rotation();
	const FVector BoxHalfSize(
		FMath::Max(PreviousHalfSize.X, CurrentHalfSize.X),
		FMath::Max(PreviousHalfSize.Y, CurrentHalfSize.Y),
		FMath::Max(PreviousHalfSize.Z, CurrentHalfSize.Z));
	

	const bool bHit = UKismetSystemLibrary::BoxTraceMulti(
		World,
		BoxTraceStart,
		BoxTraceEnd,
		BoxHalfSize,
		TraceOrientation,
		GetAttackTraceChannel(),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForOneFrame,
		// GetAttackTraceDrawDebugType(),
		OutHits,
		false,
		FLinearColor(0.0f, 1.0f, 1.0f, 1.0f),
		FLinearColor::Red,
		GetAttackTraceDebugLifetime(World)
	);

	if (!bHit)
	{
		return;
	}

	for (const FHitResult& OutHit : OutHits)
	{
		AActor* HitActor = OutHit.GetActor();
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

		const FVector Impulse = (OutHit.ImpactNormal * -MeleeKnockbackImpulse) + (FVector::UpVector * MeleeLaunchImpulse);

		Damageable->ApplyDamage(MeleeDamage, CharacterOwner, OutHit.ImpactPoint, Impulse);
		OnDamageDealt.Broadcast(MeleeDamage, OutHit.ImpactPoint);
	}
}

UStaticMeshComponent* UCombatAttackComponent::GetEquippedWeaponMesh(ACharacter* CharacterOwner) const
{
	const AAMBCharacter* CombatCharacterOwner = Cast<AAMBCharacter>(CharacterOwner);
	return CombatCharacterOwner ? CombatCharacterOwner->GetEquippedItemMeshComponent() : nullptr;
}

bool UCombatAttackComponent::BuildAttackTraceBox(
	ACharacter* CharacterOwner,
	const FVector& SegmentStart,
	const FVector& SegmentEnd,
	FVector& OutCenter,
	FRotator& OutOrientation,
	FVector& OutHalfSize) const
{
	const FVector SegmentVector = SegmentEnd - SegmentStart;
	const float SegmentLength = SegmentVector.Size();
	if (SegmentLength <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	OutCenter = (SegmentStart + SegmentEnd) * 0.5f;
	OutOrientation = SegmentVector.GetSafeNormal().Rotation();

	const FVector ThicknessHalfSize = GetAttackTraceThicknessHalfSize(CharacterOwner);
	OutHalfSize = FVector(
		FMath::Max(SegmentLength * 0.5f, 1.0f),
		ThicknessHalfSize.Y,
		ThicknessHalfSize.Z);
	return true;
}

FVector UCombatAttackComponent::GetAttackTraceThicknessHalfSize(ACharacter* CharacterOwner) const
{
	if (UStaticMeshComponent* EquippedWeaponMesh = GetEquippedWeaponMesh(CharacterOwner))
	{
		if (const UStaticMesh* EquippedStaticMesh = EquippedWeaponMesh->GetStaticMesh())
		{
			const FVector MeshScale = EquippedWeaponMesh->GetComponentScale().GetAbs();
			const FVector LocalHalfSize = EquippedStaticMesh->GetBoundingBox().GetExtent();
			const FVector ScaledHalfSize(
				LocalHalfSize.X * MeshScale.X,
				LocalHalfSize.Y * MeshScale.Y,
				LocalHalfSize.Z * MeshScale.Z);

			const float MaxExtent = FMath::Max3(ScaledHalfSize.X, ScaledHalfSize.Y, ScaledHalfSize.Z);
			const float MinExtent = FMath::Min3(ScaledHalfSize.X, ScaledHalfSize.Y, ScaledHalfSize.Z);
			const float MidExtent = (ScaledHalfSize.X + ScaledHalfSize.Y + ScaledHalfSize.Z) - MaxExtent - MinExtent;

			return FVector(
				1.0f,
				FMath::Max(MidExtent, 1.0f),
				FMath::Max(MinExtent, 1.0f));
		}
	}

	return FVector(1.0f, FMath::Max(MeleeTraceRadius * 0.35f, 1.0f), FMath::Max(MeleeTraceRadius * 0.35f, 1.0f));
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

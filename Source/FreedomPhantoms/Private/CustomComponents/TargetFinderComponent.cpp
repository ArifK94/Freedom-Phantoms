// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "Weapons/Weapon.h"
#include "Characters/BaseCharacter.h"
#include "FreedomPhantoms/FreedomPhantoms.h"
#include "Services/SharedService.h"

#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

void UTargetFinderComponent::SetFindTargetPerFrame(bool Value)
{
	FindTargetPerFrame = Value;
	SetComponentTickEnabled(Value);
}

UTargetFinderComponent::UTargetFinderComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	TargetSightRadius = 7000.0f;
	LoseSightRadius = 7500.f;
	FinderLimit = 5;

	CountdownTargetLost = 5.f;

	FindTargetPerFrame = false;
	ShowDebugTrace = false;

	ClassFilters.Add(ACharacter::StaticClass());

	IgnoreActorClasses.Add(AWeapon::StaticClass());
}


void UTargetFinderComponent::BeginPlay()
{
	Super::BeginPlay();

	Init();

	SetFindTargetPerFrame(FindTargetPerFrame);
}

void UTargetFinderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FindTarget();
}

void UTargetFinderComponent::Init()
{
	Super::Init();

	if (!GetOwningPawn()) {
		return;
	}

	// Alternative to AI Sight Perception in case 360 sight is wanted
	if (!TargetSightSphere)
	{
		TargetSightSphere = NewObject<USphereComponent>(GetOwningPawn());

		if (TargetSightSphere)
		{
			TargetSightSphere->RegisterComponent();
			TargetSightSphere->AttachToComponent(GetOwningPawn()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			TargetSightSphere->SetSphereRadius(TargetSightRadius);
			TargetSightSphere->SetCanEverAffectNavigation(false);
			TargetSightSphere->SetCollisionProfileName(TEXT("AITargetSight"));
		}
	}
}

AActor* UTargetFinderComponent::FindTarget()
{
	if (!GetOwningPawn()) {
		return nullptr;
	}

	if (!TargetSightSphere) {
		Init();

		if (!TargetSightSphere) {
			return nullptr;
		}
	}

	TArray<AActor*> OutActors = GetActorsInRadius(TargetSightRadius);

	AActor* TargetActor = nullptr;
	float TargetSightDistance = TargetSightRadius;


	FVector OwnerLocation = GetOwningPawn()->GetActorLocation();
	FVector EyeLocation;
	FRotator EyeRotation;
	GetOwningPawn()->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	// The current limit of targets to process
	int CurrentProcessedCharacters = 0;
	auto TargetSearchParameters = FTargetSearchParameters();

	for (int index = 0; index < OutActors.Num(); index++)
	{
		if (CurrentProcessedCharacters > FinderLimit) {
			break;
		}

		AActor* PotentialEnemy = OutActors[index];

		if (!UKismetSystemLibrary::IsValid(PotentialEnemy)) {
			continue;
		}

		// Do not process the same actors if already added to the ignore list so we can save performance.
		if (ProcessedIgnoreActors.Contains(PotentialEnemy->GetName())) {
			continue;
		}

		// is this the owner?
		if (PotentialEnemy == GetOwningPawn()) {
			AddToIgnoreProcessed(PotentialEnemy);
			continue;
		}

		// Filter the class
		if (!IsActorFiltered(PotentialEnemy)) {

			// If the actor has children, then the children maybe potential targets such as enemy characters attached to vehicle actors
			PotentialEnemy = GetChildrenTargets(PotentialEnemy);

			// If no children actors are potential targets then skip 
			if (!PotentialEnemy) {
				AddToIgnoreProcessed(PotentialEnemy);
				continue;
			}

		}

		auto TeamFaction = Cast<UTeamFactionComponent>(PotentialEnemy->GetComponentByClass(UTeamFactionComponent::StaticClass()));

		// Does not have a faction component or is neutral?
		if (!TeamFaction || TeamFaction->GetSelectedFaction() == TeamFaction::Neutral) {
			AddToIgnoreProcessed(PotentialEnemy);
			continue;
		}

		// ignore if friendly
		if (UTeamFactionComponent::IsFriendly(GetOwningPawn(), PotentialEnemy)) {
			AddToIgnoreProcessed(PotentialEnemy);
			continue;
		}

		// is target alive? Do not add this to ignore list as the enemy can be revived if wounded.
		if (!UHealthComponent::IsActorAlive(PotentialEnemy)) {
			continue;
		}

		FVector TargetLocation;
		bool HasHitTarget = CanSeeTarget(PotentialEnemy, TargetLocation);

		FVector EnemyLocation = PotentialEnemy->GetActorLocation();

		if (HasHitTarget)
		{
			// get closest enemy
			float DistanceDiff = FVector::Dist(OwnerLocation, EnemyLocation);

			if (DistanceDiff < TargetSightDistance)
			{
				TargetSightDistance = DistanceDiff;
				TargetActor = PotentialEnemy;
			}

			CurrentProcessedCharacters++;
		}

	}

	// Should the last seen enemy be overriden? 
	bool CanSeeLast = CanSeeLastTarget();

	// if still can see last target, is the new target closer to the owner?
	if (CanSeeLast)
	{
		// if the new target is not close by, then continue using the last enemy.
		if (TargetActor && !USharedService::IsNearTargetPosition(GetOwningPawn()->GetActorLocation(), TargetActor->GetActorLocation(), 500.f))
		{
			TargetActor = LastSeenTarget;
		}
	}

	// If new target found then clear last target.
	if (TargetActor)
	{
		ClearLastSeenTarget();
	}
	// if last target is still alive?
	else if (UHealthComponent::IsActorAlive(LastSeenTarget))
	{
		// begin countdown to clear the last target but stay on the last target until countdown has finished.
		if (!THandler_CountdownTargetLost.IsValid())
		{
			GetOwner()->GetWorldTimerManager().SetTimer(THandler_CountdownTargetLost, this, &UTargetFinderComponent::ClearLastSeenTarget, CountdownTargetLost, true);
		}

		OnTargetSearch.Broadcast(LastSeenTargetParam);
		return LastSeenTarget;
	}
	// otherwise clear the last target.
	else
	{
		ClearLastSeenTarget();
	}


	FVector TargetLoc;
	CanSeeTarget(TargetActor, TargetLoc);

	TargetSearchParameters.TargetActor = TargetActor;
	TargetSearchParameters.TargetLocation = TargetLoc;

	LastSeenTargetParam = TargetSearchParameters;
	LastSeenTarget = TargetActor;

	OnTargetSearch.Broadcast(TargetSearchParameters);
	return TargetActor;
}


bool UTargetFinderComponent::DoesClassFilterExist(TSubclassOf<AActor> Class)
{
	return ClassFilters.Contains(Class);
}

void UTargetFinderComponent::RemoveClassFilter(TSubclassOf<AActor> Class)
{
	for (int i = 0; i < ClassFilters.Num(); i++)
	{
		auto Filter = ClassFilters[i];
		if (Filter == Class)
		{
			ClassFilters.RemoveAt(i);
			break;
		}
	}
}

bool UTargetFinderComponent::CanSeeTarget(AActor* TargetActor, FVector& TargetLocation)
{
	if (TargetActor == nullptr || !UKismetSystemLibrary::IsValid(TargetActor)) {
		return false;
	}

	FVector OwnerLocation = GetOwningPawn()->GetActorLocation();
	FVector EyeLocation;
	FRotator EyeRotation;
	GetOwningPawn()->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector ActorLocation = TargetActor->GetActorLocation();

	// check if can see the target
	FHitResult HitTargetResult;
	auto Trace = GetTrace(HitTargetResult, EyeLocation, ActorLocation, TargetActor);

	if (Trace && HitTargetResult.GetActor() == TargetActor)
	{
		TargetLocation = ActorLocation;
		return true;
	}

	// if not hit the target actor yet, try line trace to the actor's head.
	auto EnemyCharacter = Cast<ABaseCharacter>(TargetActor);

	if (EnemyCharacter)
	{
		FVector EnemyEyeLocation = EnemyCharacter->GetMesh()->GetSocketLocation(EnemyCharacter->GetHeadSocket());

		// check if can see the target
		FHitResult OutHitHead;
		auto HeadTrace = GetTrace(OutHitHead, EyeLocation, EnemyEyeLocation, TargetActor);

		if (HeadTrace && OutHitHead.GetActor() == TargetActor)
		{
			TargetLocation = EnemyEyeLocation;
			return true;
		}
	}


	// if not hit the target actor yet, line trace any of its child meshes.
	auto TargetComponents = TargetActor->GetComponents();

	for (auto TargetComponent : TargetComponents)
	{
		auto SkelComp = Cast<USkeletalMeshComponent>(TargetComponent);

		if (!SkelComp) {
			continue;
		}

		// Find a random bone to trace rather than trace each bone to save a bit of performance. 
		// Some bones can be trasnformed away from the character mesh and so the trace will return nothing about the target.
		auto BoneIndex = FMath::RandRange(0, SkelComp->GetNumBones());
		FHitResult OutHitComponent;
		auto CompTrace = GetTrace(OutHitComponent, EyeLocation, SkelComp->GetBoneLocation(SkelComp->GetBoneName(BoneIndex)), TargetActor);

		if (CompTrace && (HitTargetResult.GetComponent() == TargetComponent || HitTargetResult.GetActor() == TargetActor))
		{
			TargetLocation = SkelComp->GetBoneLocation(SkelComp->GetBoneName(BoneIndex));
			return true;
		}
	}

	return false;
}

AActor* UTargetFinderComponent::GetChildrenTargets(AActor* ParentTarget)
{
	AActor* PotentialTarget = nullptr;

	if (!ParentTarget) {
		return PotentialTarget;
	}

	TArray<AActor*> ChildAttachedActors;
	ParentTarget->GetAttachedActors(ChildAttachedActors);


	if (ChildAttachedActors.Num() <= 0) {
		return nullptr;
	}

	for (int i = 0; i < ChildAttachedActors.Num(); i++)
	{
		auto Actor = ChildAttachedActors[i];

		if (Actor == GetOwningPawn()) {
			continue;
		}

		if (IsActorFiltered(Actor))
		{
			return Actor;
		}
	}

	return PotentialTarget;
}

bool UTargetFinderComponent::IsActorFiltered(AActor* Actor)
{
	if (ClassFilters.Num() <= 0 || !Actor) {
		return true;
	}

	for (int i = 0; i < ClassFilters.Num(); i++)
	{
		if (Actor->IsA(ClassFilters[i])) {
			return true;
		}
	}

	return false;
}

bool UTargetFinderComponent::IsActorToIgnore(AActor* Actor)
{
	if (IgnoreActorClasses.Num() <= 0 || !Actor) {
		return true;
	}

	for (int i = 0; i < IgnoreActorClasses.Num(); i++)
	{
		if (Actor->IsA(IgnoreActorClasses[i])) {
			return true;
		}
	}

	return false;
}

void UTargetFinderComponent::ClearLastSeenTarget()
{
	LastSeenTarget = nullptr;

	if (GetOwningPawn() && GetWorld()) 
	{
		GetOwner()->GetWorldTimerManager().ClearTimer(THandler_CountdownTargetLost);
	}
}

void UTargetFinderComponent::AddToIgnoreProcessed(AActor* Actor)
{
	if (Actor && !ProcessedIgnoreActors.Contains(Actor->GetName()))
	{
		ProcessedIgnoreActors.Add(Actor->GetName());
	}
}

bool UTargetFinderComponent::GetTrace(FHitResult& OutHit, FVector Start, FVector End, AActor* TargetActor)
{
	auto Owner = GetOwningPawn();
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(Owner);

	// Ignore specified actors
	for (int i = 0; i < IgnoreActors.Num(); i++)
	{
		auto Actor = IgnoreActors[i];

		if (Actor) {
			QueryParams.AddIgnoredActor(Actor);
		}
	}

	FCollisionObjectQueryParams ObjectParams;
	for (auto Iter = CollisionChannels.CreateConstIterator(); Iter; ++Iter)
	{
		ObjectParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel((*Iter).GetValue()));
	}

	if (ShowDebugTrace)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.f);
	}

	// Multi line trace is required as certain actor classes need to be ignored & single trace does not provide functionality to ignore actor classes
	TArray<FHitResult> OutHits;
	auto IsHit = GetWorld()->LineTraceMultiByChannel(
		OutHits,
		Start,
		End,
		TRACE_SIGHT,
		QueryParams
	);

	for (auto HitData : OutHits)
	{
		auto HitActor = HitData.GetActor();
		if (HitActor)
		{
			// If actor hit is not to be ignored then return this hit data
			if (!IsActorToIgnore(HitActor) && HitActor != Owner)
			{
				OutHit = HitData;

				if (ShowDebugTrace)
				{
					DrawDebugLine(GetWorld(), OutHit.ImpactPoint, End, FColor::Red, false, 2.f);
				}

				if (OutHit.GetActor() == TargetActor)
				{
					return true;
				}
			}
		}
	}


	return IsHit;
}


void UTargetFinderComponent::AddIgnoreClass(AActor* InOwner, TSubclassOf<AActor> InClass)
{
	if (!InOwner || !InClass) {
		return;
	}

	auto ActorComponent = InOwner->GetComponentByClass(UTargetFinderComponent::StaticClass());

	if (ActorComponent) {
		auto TargetFinderComp = Cast<UTargetFinderComponent>(ActorComponent);

		if (TargetFinderComp)
		{
			TargetFinderComp->AddIgnoreClass(InClass);
		}
	}
}

void UTargetFinderComponent::AddIgnoreActor(AActor* InOwner, AActor* InActorIgnore)
{
	if (!InOwner || !InActorIgnore) {
		return;
	}

	auto ActorComponent = InOwner->GetComponentByClass(UTargetFinderComponent::StaticClass());

	if (ActorComponent) {
		auto TargetFinderComp = Cast<UTargetFinderComponent>(ActorComponent);

		if (TargetFinderComp)
		{
			TargetFinderComp->AddIgnoreActor(InActorIgnore);
		}
	}
}

FRotator UTargetFinderComponent::RotateTowardsTarget(AActor* OwnerActor, AActor* TargetActor, FRotator CurrentRotation, FRotator& TargetRotation, float DeltaTime, float LerpSpeed)
{
	if (!TargetActor)
	{
		TargetRotation = FRotator::ZeroRotator;
	}
	else
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		OwnerActor->GetActorEyesViewPoint(EyeLocation, EyeRotation); // Grab actor eye viewpoint

		auto TargetLocation = TargetActor->GetActorLocation() - EyeLocation;
		TargetRotation = UKismetMathLibrary::MakeRotFromX(TargetLocation);
	}

	return UKismetMathLibrary::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, LerpSpeed);
}


bool UTargetFinderComponent::IsTargetBehind(AActor* ActorA, AActor* TargetActor)
{
	if (ActorA == nullptr || TargetActor == nullptr) {
		return false;
	}

	FVector MGForwardPos = UKismetMathLibrary::GetForwardVector(ActorA->GetActorRotation());
	FVector Normalised = TargetActor->GetActorLocation() - ActorA->GetActorLocation();
	UKismetMathLibrary::Vector_Normalize(Normalised);
	float Angle = UKismetMathLibrary::Dot_VectorVector(MGForwardPos, Normalised);

	if (Angle < -0.7f)
	{
		return true;
	}

	return false;
}

TArray<AActor*> UTargetFinderComponent::GetActorsInRadius(float Radius)
{
	TArray<AActor*> OutActors;

	// Get all overlapped actors based that have team faction component attached
	TargetSightSphere->GetOverlappingActors(OutActors, AActor::StaticClass());

	return OutActors;
}

bool UTargetFinderComponent::CanSeeLastTarget()
{
	if (LastSeenTarget == nullptr) {
		return false;
	}

	FVector TargetLocation;
	bool HasHitTarget = CanSeeTarget(LastSeenTarget, TargetLocation);

	if (!HasHitTarget) {
		return false;
	}


	if (!UHealthComponent::IsActorAlive(LastSeenTarget)) {
		return false;
	}

	return true;
}
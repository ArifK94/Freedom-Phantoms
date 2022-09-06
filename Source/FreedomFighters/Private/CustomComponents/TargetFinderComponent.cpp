// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "Weapons/Weapon.h"
#include "Characters/BaseCharacter.h"
#include "FreedomFighters/FreedomFighters.h"
#include "Services/SharedService.h"

#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

void UTargetFinderComponent::SetFindTargetPerFrame(bool Value)
{
	if (Value)
	{
		if (!THandler_TargetSearch.IsValid())
		{
			GetOwner()->GetWorldTimerManager().SetTimer(THandler_TargetSearch, this, &UTargetFinderComponent::FindTargetUpdate, 1.f, true);
		}
	}
	else
	{
		if (THandler_TargetSearch.IsValid())
		{
			GetOwner()->GetWorldTimerManager().ClearTimer(THandler_TargetSearch);
		}
	}

	FindTargetPerFrame = Value;
}

UTargetFinderComponent::UTargetFinderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	TargetSightRadius = 5000.0f;
	FinderLimit = 5;

	FindTargetPerFrame = false;
	CreateTargetSphere = true;
	ShowDebugTrace = false;

	ClassFilters.Add(ACharacter::StaticClass());

	IgnoreActorClasses.Add(AWeapon::StaticClass());
}


void UTargetFinderComponent::BeginPlay()
{
	Super::BeginPlay();

	Init();
}

void UTargetFinderComponent::Init()
{
	if (!GetOwner()) {
		return;
	}

	// Alternative to AI Sight Perception in case 360 sight is wanted
	if (CreateTargetSphere && TargetSightSphere == nullptr)
	{
		TargetSightSphere = NewObject<USphereComponent>(GetOwner());

		if (TargetSightSphere)
		{
			TargetSightSphere->RegisterComponent();
			TargetSightSphere->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			TargetSightSphere->SetSphereRadius(TargetSightRadius);
			TargetSightSphere->SetCanEverAffectNavigation(false);
			TargetSightSphere->SetCollisionProfileName(TEXT("AITargetSight"));
		}
	}

	if (FindTargetPerFrame)
	{
		if (!THandler_TargetSearch.IsValid())
		{
			GetOwner()->GetWorldTimerManager().SetTimer(THandler_TargetSearch, this, &UTargetFinderComponent::FindTargetUpdate, 1.f, true);
		}
	}
}

void UTargetFinderComponent::FindTargetUpdate()
{
	FindTarget();
}

bool UTargetFinderComponent::CanSeeLastTarget()
{
	if (LastSeenEnemy == nullptr) {
		return false;
	}

	FVector TargetLocation;
	bool HasHitTarget = CanSeeTarget(LastSeenEnemy, TargetLocation);

	if (!HasHitTarget) {
		return false;
	}


	if (!UHealthComponent::IsAlive(LastSeenEnemy)) {
		return false;
	}

	return true;
}

AActor* UTargetFinderComponent::FindTarget()
{
	if (!GetOwner()) {
		return nullptr;
	}

	if (!TargetSightSphere) {
		Init();

		if (!TargetSightSphere) {
			return nullptr;
		}
	}

	AActor* TargetActor = nullptr;
	TArray<AActor*> ActorsInSight;
	float TargetSightDistance = TargetSightRadius;

	// Get all overlapped actors based that have team faction component attached
	TargetSightSphere->GetOverlappingActors(ActorsInSight, AActor::StaticClass());

	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FVector EyeLocation;
	FRotator EyeRotation;
	GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	// The current limit of targets to process
	int CurrentProcessedCharacters = 0;
	auto TargetSearchParameters = FTargetSearchParameters();

	for (int index = 0; index < ActorsInSight.Num(); index++)
	{
		if (CurrentProcessedCharacters > FinderLimit) {
			break;
		}

		AActor* PotentialEnemy = ActorsInSight[index];

		if (!PotentialEnemy) {
			continue;
		}

		// is this the owner?
		if (PotentialEnemy == GetOwner()) {
			continue;
		}

		// Filter the class
		if (!IsActorFiltered(PotentialEnemy)) {

			// If the actor has children, then the children maybe potential targets such as enemy characters attached to vehicle actors
			PotentialEnemy = GetChildrenTargets(PotentialEnemy);

			// If no children actors are potential targets then skip 
			if (!PotentialEnemy) {
				continue;
			}

		}

		auto TeamFaction = Cast<UTeamFactionComponent>(PotentialEnemy->GetComponentByClass(UTeamFactionComponent::StaticClass()));

		// Does not have a faction component or is neutral?
		if (!TeamFaction || TeamFaction->GetSelectedFaction() == TeamFaction::Neutral) {
			continue;
		}

		bool IsAlive = UHealthComponent::IsAlive(PotentialEnemy);

		// is target alive
		if (!IsAlive) {
			continue;
		}

		bool IsFriendly = UTeamFactionComponent::IsFriendly(GetOwner(), PotentialEnemy);

		// ignore if friendly
		if (IsFriendly) {
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
		if (TargetActor && !SharedService::IsNearTargetPosition(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation(), 500.f))
		{
			TargetActor = LastSeenEnemy;
		}
	}

	LastSeenEnemy = TargetActor;
	TargetSearchParameters.TargetActor = TargetActor;
	OnTargetSearch.Broadcast(TargetSearchParameters);
	return TargetActor;
}


bool UTargetFinderComponent::DoesClassFilterExist(TSubclassOf<AActor> Class)
{
	return ClassFilters.Contains(Class);
}

bool UTargetFinderComponent::CanSeeTarget(AActor* TargetActor, FVector& TargetLocation)
{
	if (TargetActor == nullptr) {
		return false;
	}

	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FVector EyeLocation;
	FRotator EyeRotation;
	GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector ActorLocation = TargetActor->GetActorLocation();

	// check if can see the target
	FHitResult HitTargetResult;
	auto Trace = GetTrace(HitTargetResult, EyeLocation, ActorLocation);


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
		FRotator EnemyEyeRotation;
		TargetActor->GetActorEyesViewPoint(EnemyEyeLocation, EnemyEyeRotation);

		// check if can see the target
		FHitResult OutHitHead;
		auto HeadTrace = GetTrace(OutHitHead, EyeLocation, EnemyEyeLocation);

		if (HeadTrace && OutHitHead.GetActor() == TargetActor)
		{
			TargetLocation = EnemyEyeLocation;
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

		if (Actor == GetOwner()) {
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

bool UTargetFinderComponent::GetTrace(FHitResult& OutHit, FVector Start, FVector End)
{
	auto Owner = GetOwner();
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
	//auto IsHit = GetWorld()->LineTraceMultiByObjectType(
	//	OutHits,
	//	Start,
	//	End,
	//	ObjectParams,
	//	QueryParams
	//);

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
		//auto RootBone = MeshComp->GetBoneName(0);
		//auto TargetDirectionInvert = UKismetMathLibrary::InverseTransformDirection(MeshComp->GetSocketTransform(RootBone), TargetLocation);
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
// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/AIMovementComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "Characters/BaseCharacter.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

UAIMovementComponent::UAIMovementComponent()
{
	MinAcceptanceRadius = 50.f;
	MovementDebugLifeTime = 1.0f;
	ProjectDestinationToNavigation = true;

	UsePathfinding = true;
}

void UAIMovementComponent::Init()
{
	Super::Init();

	if (GetController())
	{
		AIController = Cast<AAIController>(GetController());
	}
}

EPathFollowingRequestResult::Type UAIMovementComponent::MoveToDestination(FVector TargetDestination, float AcceptRadius, AIBehaviourState BehaviourState, bool SprintToTarget, bool WalkNearTarget)
{
	if (!AIController) {
		Init();
	}

	if (!AIController || TargetDestination.IsZero()) {
		CurrentMovement = EPathFollowingRequestResult::Failed;
		return CurrentMovement;
	}

	CurrentMovement = AIController->MoveToLocation(TargetDestination, AcceptRadius, StopOnOverlap, UsePathfinding, ProjectDestinationToNavigation, CanStrafe, FilterClass, AllowPartialPaths);

	if (CurrentMovement != EPathFollowingRequestResult::RequestSuccessful) {
		return CurrentMovement;
	}

	// Make sure sphere radius is not very small otherwise the overlap will never trigger
	// Set a small random amount
	// otherwise use the accept radius amount
	float TargetRadius = AcceptRadius <= 1.f ? MinAcceptanceRadius : AcceptRadius;


	if (!GetOwningCharacter()) {
		Init();
	}

	//DrawDebugSphere(GetWorld(), TargetDestination, TargetRadius, 20, FColor::Purple, false, 20.f, 0, 2);

	if (GetOwningCharacter() && SprintToTarget)
	{
		// break off cover.
		if (GetOwningCharacter()->IsTakingCover()) {
			GetOwningCharacter()->StopCover();
		}

		// Walk when close to desination
		if (WalkNearTarget)
		{
			FVector OwnerLocation = GetOwningCharacter()->GetActorLocation();

			float CurrentTargetDistance = UKismetMathLibrary::Vector_Distance(OwnerLocation, TargetDestination);

			// if distance is outside of destination radius
			if (CurrentTargetDistance > TargetRadius)
			{
				GetOwningCharacter()->BeginSprint();	// sprint
			}
			else
			{
				GetOwningCharacter()->EndSprint();
			}
		}
		else
		{
			if (CurrentMovement != EPathFollowingRequestResult::AlreadyAtGoal)
			{
				GetOwningCharacter()->BeginSprint();
			}
			else
			{
				GetOwningCharacter()->EndSprint();
			}
		}
	}

	OnDestinationSet.Broadcast(BehaviourState);

	return CurrentMovement;
}

FVector UAIMovementComponent::FindNearbyDestinationPoint(FVector TargetDestination, float Radius, TArray<AActor*> IgnoreActors)
{
	// Assign default target by the destination set
	FVector TargetDest = TargetDestination;

	// create a collision sphere
	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(Radius);

	// create tarray for hit results
	TArray<FHitResult> OutHits;

	IgnoreActors.Add(GetOwningCharacter());

	// check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, TargetDest, TargetDest, FQuat::Identity, ECC_Visibility, MyColSphere);


	if (isHit)
	{
		// Is there already a GetOwningCharacter() in that target dest?
		bool IsPointTaken = false;
		for (auto& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor)
			{
				if (UHealthComponent::IsActorAlive(HitActor))
				{
					// hit actor is meant to be ignored then ignore this.
					if (!IgnoreActors.Contains(HitActor))
					{
						IsPointTaken = true;
					}
				}
			}
		}

		// find another point in a nav radius if point is taken by another
		if (IsPointTaken)
		{
			FNavLocation NavLocation;
			UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
			bool navResult = NavigationArea->GetRandomReachablePointInRadius(TargetDestination, Radius, NavLocation);

			if (navResult)
			{
				TargetDest = NavLocation.Location;
			}

		}
	}

	return TargetDest;
}

FVector UAIMovementComponent::ValidateDestination(FVector Location, bool& IsLocationValid)
{
	FVector Destination;
	FNavLocation NavLocation;
	UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	bool navResult = NavigationArea->ProjectPointToNavigation(Location, NavLocation, Destination);

	IsLocationValid = navResult;

	if (navResult)
	{
		Destination = NavLocation.Location;
	}

	return Destination;
}
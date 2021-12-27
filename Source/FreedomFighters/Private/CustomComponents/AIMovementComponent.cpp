// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/AIMovementComponent.h"
#include "Characters/BaseCharacter.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"

UAIMovementComponent::UAIMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UAIMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	Init();
}

void UAIMovementComponent::Init()
{
	if (!GetOwner()) {
		return;
	}

	AIController = Cast<AAIController>(GetOwner());

	if (AIController)
	{
		auto Pawn = AIController->GetPawn();

		if (Pawn)
		{
			Character = Cast<ABaseCharacter>(Pawn);
		}
	}

}


EPathFollowingRequestResult::Type UAIMovementComponent::MoveToDestination(FVector TargetDestination, float AcceptRadius, bool WalkNearTarget)
{
	auto CurrentMovement = EPathFollowingRequestResult::Failed;

	if (!AIController)
	{
		Init();
	}

	if (TargetDestination.IsZero() || !AIController) {
		return CurrentMovement;
	}

	CurrentMovement = AIController->MoveToLocation(TargetDestination, AcceptRadius, StopOnOverlap, UsePathfinding, ProjectDestinationToNavigation, CanStrafe, FilterClass, AllowPartialPaths);

	if (Character)
	{
		// Walk when close to desination
		if (WalkNearTarget)
		{
			FVector OwnerLocation = Character->GetActorLocation();

			float CurrentTargetDistance = UKismetMathLibrary::Vector_Distance(OwnerLocation, TargetDestination);

			if (CurrentTargetDistance > AcceptanceRadius)
			{
				Character->BeginSprint();
			}
			else
			{
				Character->EndSprint();
			}
		}
		else
		{
			if (CurrentMovement != EPathFollowingRequestResult::AlreadyAtGoal)
			{
				Character->BeginSprint();
			}
			else
			{
				Character->EndSprint();
			}
		}
	}

	return CurrentMovement;
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
// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/AIMovementComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "Characters/BaseCharacter.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"

UAIMovementComponent::UAIMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MinAcceptanceRadius = 50.f;
	MovementDebugLifetTime = 1.0f;
	ProjectDestinationToNavigation = true;

	IsDestinationSet = false;
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

	if (!AIController) {
		return;
	}
	PawnOwner = AIController->GetPawn();

	if (PawnOwner)
	{
		Character = Cast<ABaseCharacter>(PawnOwner);

		if (!DestinationTrigger)
		{
			DestinationTrigger = NewObject<USphereComponent>(this);
			DestinationTrigger->RegisterComponent();
			DestinationTrigger->SetSphereRadius(.0f);
			DestinationTrigger->SetWorldLocation(FVector::ZeroVector);
			DestinationTrigger->SetCollisionProfileName(TEXT("OverlapAllCharacter"));
			DestinationTrigger->ShapeColor = FColor(0, 255, 0, 255);
			DestinationTrigger->OnComponentBeginOverlap.AddDynamic(this, &UAIMovementComponent::OnOverlapBegin);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No Pawn Owner for the AI movement component"));
	}
}

void UAIMovementComponent::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !PawnOwner || !IsDestinationSet) {
		return;
	}

	if (OtherActor == PawnOwner)
	{
		OnDestinationReached.Broadcast(DestinationTrigger->GetComponentLocation());
		IsDestinationSet = false;
		DestinationTrigger->SetSphereRadius(.0f); // set radius to zero, this allows the being overlap to trigger again if the next destination is within sphere radius
	}
}


EPathFollowingRequestResult::Type UAIMovementComponent::MoveToDestination(FVector TargetDestination, float AcceptRadius, bool SprintToTarget, bool WalkNearTarget, AIBehaviourState BehaviourState)
{
	auto CurrentMovement = EPathFollowingRequestResult::Failed;

	if (!AIController || !DestinationTrigger) {
		Init();
	}

	if (!AIController || !DestinationTrigger || TargetDestination.IsZero()) {
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

	DestinationTrigger->SetWorldLocation(TargetDestination);
	DestinationTrigger->SetSphereRadius(TargetRadius);
	IsDestinationSet = true;


	if (Character && SprintToTarget)
	{
		// Walk when close to desination
		if (WalkNearTarget)
		{
			FVector OwnerLocation = Character->GetActorLocation();

			float CurrentTargetDistance = UKismetMathLibrary::Vector_Distance(OwnerLocation, TargetDestination);

			// if distance is outside of destination radius
			if (CurrentTargetDistance > TargetRadius)
			{
				Character->BeginSprint();	// sprint
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

	// check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, TargetDest, TargetDest, FQuat::Identity, ECC_Visibility, MyColSphere);


	if (isHit)
	{
		// Is there already a character in that target dest?
		bool IsPointTaken = false;
		for (auto& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor)
			{
				if (UHealthComponent::IsAlive(HitActor))
				{
					for (int i = 0; i < IgnoreActors.Num(); i++)
					{
						auto Actor = IgnoreActors[i];

						if (Actor != HitActor)
						{
							IsPointTaken = true;
						}
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
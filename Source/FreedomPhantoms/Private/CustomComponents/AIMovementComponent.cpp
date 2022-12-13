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
	PrimaryComponentTick.bCanEverTick = false;

	MinAcceptanceRadius = 50.f;
	MovementDebugLifetTime = 1.0f;
	ProjectDestinationToNavigation = true;

	UsePathfinding = true;
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
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No Pawn Owner for the AI movement component"));
	}
}

void UAIMovementComponent::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !PawnOwner) {
		return;
	}


	if (OtherActor == PawnOwner)
	{
		auto TriggerLocation = OtherActor->GetActorLocation();

		//	DrawDebugSphere(GetWorld(), TriggerLocation, 30.f, 20, FColor::Red, false, 20.f, 0, 2);

		if (DestinationTrigger) {

			DestinationTrigger->SetCollisionProfileName(TEXT("NoCollision"));

			//if (DestinationTrigger->OnComponentBeginOverlap.IsBound()) {
			//	DestinationTrigger->OnComponentBeginOverlap.RemoveDynamic(this, &UAIMovementComponent::OnOverlapBegin);
			//}

			//DestinationTrigger->DestroyComponent();
		}

		OnDestinationReached.Broadcast(TriggerLocation);
	}
}


EPathFollowingRequestResult::Type UAIMovementComponent::MoveToDestination(FVector TargetDestination, float AcceptRadius, AIBehaviourState BehaviourState, bool SprintToTarget, bool WalkNearTarget)
{
	auto CurrentMovement = EPathFollowingRequestResult::Failed;

	if (!AIController) {
		Init();
	}

	if (!AIController || TargetDestination.IsZero()) {
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


	CreateDestinationTrigger(TargetDestination, TargetRadius);

	if (DestinationTrigger) {
		DestinationTrigger->SetCollisionProfileName(TEXT("OverlapAllCharacter"));
	}

	if (Character == nullptr) {
		Init();
	}

	//DrawDebugSphere(GetWorld(), TargetDestination, TargetRadius, 20, FColor::Purple, false, 20.f, 0, 2);

	if (Character && SprintToTarget)
	{
		// break off cover.
		if (Character->IsTakingCover()) {
			Character->StopCover();
		}

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

	IgnoreActors.Add(Character);

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

void UAIMovementComponent::CreateDestinationTrigger(FVector Location, float Radius)
{
	if (DestinationTrigger == nullptr) {
		return;
	}

	DestinationTrigger = NewObject<USphereComponent>(this);
	DestinationTrigger->RegisterComponent();
	DestinationTrigger->SetSphereRadius(Radius);
	DestinationTrigger->SetWorldLocation(Location);
	DestinationTrigger->SetCollisionProfileName(TEXT("OverlapAllCharacter"));
	DestinationTrigger->SetCanEverAffectNavigation(false);
	DestinationTrigger->ShapeColor = FColor(0, 255, 0, 255);

	if (!DestinationTrigger->OnComponentBeginOverlap.IsBound()) {
		DestinationTrigger->OnComponentBeginOverlap.AddDynamic(this, &UAIMovementComponent::OnOverlapBegin);
	}
}
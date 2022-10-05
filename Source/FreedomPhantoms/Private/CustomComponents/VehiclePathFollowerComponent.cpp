// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/VehiclePathFollowerComponent.h"
#include "Components/SplineComponent.h"
#include "Props/VehicleSplinePath.h"
#include "Accessories/Rope.h"
#include "Vehicles/VehicleBase.h"
#include "Characters/BaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"

UVehiclePathFollowerComponent::UVehiclePathFollowerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	PathFollowDuration = 10.0f;
	TotalLaps = 0;

	RandomNavPointRadius = 1000.f;

	FollowTargetDestination = false;
	HasInifiteLaps = false;
	DestroyOnPathComplete = true;
	FindPathPerFrame = false;

	SpawnLocationMin = FVector(-10.f, -10.f, 10.f);
	SpawnLocationMax = FVector(10.f, 10.f, 10.f);
}

void UVehiclePathFollowerComponent::Init()
{
	if (CollisionDetector == nullptr)
	{
		CollisionDetector = NewObject<UCapsuleComponent>(GetOwner());

		if (CollisionDetector)
		{
			CollisionDetector->RegisterComponent();
			CollisionDetector->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			CollisionDetector->SetCollisionProfileName(TEXT("OverlapAll"));
			CollisionDetector->SetCanEverAffectNavigation(false);
			CollisionDetector->bDynamicObstacle = true;
			CollisionDetector->CanCharacterStepUpOn = ECB_No;
			CollisionDetector->OnComponentBeginOverlap.AddDynamic(this, &UVehiclePathFollowerComponent::OnOverlapBegin);
		}
	}

	if (OwningVehicle == nullptr)
	{
		OwningVehicle = Cast<AVehicleBase>(GetOwner());
	}

	DefaultPathFollowDuration = PathFollowDuration;

	FindPath();
}

void UVehiclePathFollowerComponent::BeginPlay()
{
	Super::BeginPlay();

	Init();

	// Call after spawning passengers, otherwise character movement component will force characters to be on the ground
	SpawnRandomLocation();

	PreviousActorLocation = GetOwner()->GetActorLocation();
}


void UVehiclePathFollowerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CurveTimeline.TickTimeline(DeltaTime);

	if (VehiclePath == nullptr && FindPathPerFrame && !IgnoreOccupiedPath)
	{
		FindPath();
	}

	auto Distance = GetOwner()->GetActorLocation() - PreviousActorLocation;
	auto Velocity = UKismetMathLibrary::Divide_VectorFloat(Distance, DeltaTime);
	GetOwner()->GetRootComponent()->ComponentVelocity = Velocity + CurveTimeline.GetPlayRate();
	PreviousActorLocation = GetOwner()->GetActorLocation();

	IsStopped = !CurveTimeline.IsPlaying();
}

void UVehiclePathFollowerComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (VehiclePath == nullptr || OtherActor == nullptr) {
		return;
	}

	auto CollidedPath = Cast<AVehicleSplinePath>(OtherActor);

	if (CollidedPath == nullptr) {
		return;
	}

	// if collided with another aircraft path then ignore then do not proceed
	if (CollidedPath != VehiclePath) {
		return;
	}

	// find current spline point
	auto NewSplineIndex = CollidedPath->GetVehicleSplinePoint(GetOwner()->GetActorLocation());

	// if current spline point not found 
	if (NewSplineIndex == -1) {
		return;
	}

	// If you reached the same spline point then do not process this point again
	if (ProcessedPoints.Contains(NewSplineIndex)) {
		return;
	}
	ProcessedPoints.Add(NewSplineIndex);

	CurrentSplinePoint = CollidedPath->GetVehicleSplinePoints()[NewSplineIndex];

	// adjust path duration to change speed if specified
	if (CurrentSplinePoint.AffectSpeedType == EVehicleSpeedType::Specified) {
		CurveTimeline.SetPlayRate(1.0f / CurrentSplinePoint.PathDuration);
	}
	else {
		CurveTimeline.SetPlayRate(1.0f / PathFollowDuration);
	}

	if (CurrentSplinePoint.IsPathFreeToUse) {
		VehiclePath->SetOccupantVehicle(nullptr);
	}

	// update the vehicle movement type
	CurrentVehicleMovement = CurrentSplinePoint.MovementType;

	switch (CurrentVehicleMovement)
	{
		// Wait at current point
	case EVehicleMovement::Waiting:
		// Stop moving
		CurveTimeline.Stop();

		// Start timer & set the delay based on the duration
		GetOwner()->GetWorldTimerManager().SetTimer(THandler_ResumePath, this, &UVehiclePathFollowerComponent::ResumePath, 1.f, false, CurrentSplinePoint.WaitingDuration);
		break;
		// Wait for passengers to leave
	case EVehicleMovement::PassengerExit:
		// Stop moving
		CurveTimeline.Stop();

		SpawnRope();

		GetOwner()->GetWorldTimerManager().SetTimer(THandler_ExitPassenger, this, &UVehiclePathFollowerComponent::ExitPassengers, 1.f, true);

		break;
	}
}

void UVehiclePathFollowerComponent::FindPath()
{
	AVehicleSplinePath* ClosestPath = nullptr;

	// if already assigned a path then return
	if (VehiclePath == nullptr)
	{
		float ClosestDistance = 0.0f;
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), VehiclePathTagName, OutActors);

		for (AActor* Actor : OutActors)
		{
			auto Path = Cast<AVehicleSplinePath>(Actor);

			// check if path is not occupied
			if (!Path || !IgnoreOccupiedPath && Path->GetOccupiedVehicle()) {
				continue;
			}

			float Distance = UKismetMathLibrary::Vector_Distance(TargetDestination, Path->GetActorLocation());

			if (ClosestPath == nullptr)
			{
				ClosestPath = Path;
				ClosestDistance = Distance;
			}
			else
			{
				if (Distance < ClosestDistance)
				{
					ClosestPath = Path;
					ClosestDistance = Distance;
				}
			}
		}
	}
	else
	{
		ClosestPath = VehiclePath;
	}
	
	VehiclePath = ClosestPath;


	if (VehiclePath)
	{
		VehiclePath->SetOccupantVehicle(GetOwner());

		// setup time line for following the path
		if (CurveFloat)
		{
			if (TransitionToSplineStart)
			{
				StartPath("MoveToSplinePathStart");
			}
			else
			{
				StartPath("FollowSplinePath");
			}

		}
	}
}

void UVehiclePathFollowerComponent::StartPath(FString PathMethodName)
{
	CurveTimeline = FTimeline();
	FOnTimelineFloat TimelineProgress;
	TimelineProgress.BindUFunction(this, FName(PathMethodName));
	CurveTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
	CurveTimeline.SetLooping(false);
	CurveTimeline.SetPlayRate(1.0f / PathFollowDuration);
	CurveTimeline.PlayFromStart();
}

void UVehiclePathFollowerComponent::FollowSplinePath(float Value)
{
	USplineComponent* SplinePathComp = VehiclePath->GetSplinePathComp();
	float Alpha = UKismetMathLibrary::Lerp(0.0f, SplinePathComp->GetSplineLength(), Value);

	FVector TargetLocation = SplinePathComp->GetLocationAtDistanceAlongSpline(Alpha, ESplineCoordinateSpace::World);
	FRotator TargetRotation = SplinePathComp->GetRotationAtDistanceAlongSpline(Alpha, ESplineCoordinateSpace::World);

	GetOwner()->SetActorLocationAndRotation(TargetLocation, TargetRotation);

	// if reached the end
	if (Alpha >= SplinePathComp->GetSplineLength())
	{
		if (CurrentLap < TotalLaps || HasInifiteLaps) // restart the lap if laps remaining / infinite
		{
			CurrentLap++;

			// reset the processed points
			ProcessedPoints.Empty();

			CurveTimeline.PlayFromStart();
		}
		else
		{
			if (DestroyOnPathComplete)
			{
				GetOwner()->Destroy();
			}

			OnPathComplete.Broadcast(OwningVehicle);
		}
	}
}

void UVehiclePathFollowerComponent::MoveToSplinePathStart(float Value)
{
	USplineComponent* SplinePathComp = VehiclePath->GetSplinePathComp();

	FVector TargetLocation = SplinePathComp->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	FRotator TargetRotation = SplinePathComp->GetRotationAtSplinePoint(0, ESplineCoordinateSpace::World);

	FVector TargetLocationLerp = UKismetMathLibrary::VLerp(GetOwner()->GetActorLocation(), TargetLocation, Value);
	FRotator TargetRotationLerp = UKismetMathLibrary::RLerp(GetOwner()->GetActorRotation(), TargetRotation, Value, true);

	GetOwner()->SetActorLocationAndRotation(TargetLocationLerp, TargetRotationLerp);

	// if reached the end
	if (UKismetMathLibrary::EqualEqual_VectorVector(GetOwner()->GetActorLocation(), TargetLocation, 20.f))
	{
		StartPath("FollowSplinePath");
	}
}

void UVehiclePathFollowerComponent::MoveToLocation(float Value)
{
	float TargetX = UKismetMathLibrary::Lerp(GetOwner()->GetActorLocation().X, TargetDestination.X, Value);
	float TargetY = UKismetMathLibrary::Lerp(GetOwner()->GetActorLocation().Y, TargetDestination.Y, Value);

	GetOwner()->SetActorLocation(FVector(TargetX, TargetY, GetOwner()->GetActorLocation().Z));

	if (Value >= 1.f)
	{
		CurrentVehicleMovement = TargetLocReachedVehicleMovementType;
	}
}

void UVehiclePathFollowerComponent::SpawnRandomLocation()
{
	FindNearestNav();

	// Set for random spawn location from target location
	if (FollowTargetDestination)
	{
		float X = UKismetMathLibrary::RandomFloatInRange(SpawnLocationMin.X * TargetDestination.X, SpawnLocationMax.X * TargetDestination.X);
		float Y = UKismetMathLibrary::RandomFloatInRange(SpawnLocationMin.Y * TargetDestination.Y, SpawnLocationMax.Y * TargetDestination.Y);
		float Z = UKismetMathLibrary::RandomFloatInRange(SpawnLocationMin.Z * TargetDestination.Z, SpawnLocationMax.Z * TargetDestination.Z);

		GetOwner()->SetActorLocation(FVector(X, Y, Z));

		FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(), TargetDestination);

		// Look towards target location
		GetOwner()->SetActorRotation(TargetRot);
	}
}

void UVehiclePathFollowerComponent::FindNearestNav()
{
	FVector TargetDest = TargetDestination;
	FNavLocation NavLocation;
	UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	bool navResult = NavigationArea->GetRandomReachablePointInRadius(TargetDest, RandomNavPointRadius, NavLocation);

	if (navResult)
	{
		TargetDestination = NavLocation.Location;
	}
}

// Free up the path for another vehicle to use
void UVehiclePathFollowerComponent::ClearPath()
{
	if (VehiclePath && VehiclePath->GetOccupiedVehicle() == GetOwner())
	{
		VehiclePath->SetOccupantVehicle(nullptr);
		GetOwner()->GetWorldTimerManager().ClearTimer(THandler_ExitPassenger);
		GetOwner()->GetWorldTimerManager().ClearTimer(THandler_ResumePath);
	}
}


void UVehiclePathFollowerComponent::ExitPassengers()
{
	auto VehicleSeats = OwningVehicle->GetVehicleSeatPtrList();

	if (VehicleSeats.Num() <= 0) {
		return;
	}

	bool HasRemainingPassengers = false;

	for (int i = 0; i < VehicleSeats.Num(); i++)
	{
		auto VehicleSeat = VehicleSeats[i];
		auto Character = VehicleSeat->Character;

		if (!Character) {
			OwningVehicle->RemovePassenger(i);
			continue;
		}

		if (!VehicleSeat->OwningVehicle) {
			OwningVehicle->RemovePassenger(i);
			continue;
		}

		// Cannot exit from vehicle?
		if (!VehicleSeat->ExitPassengerOnPoint) {
			continue;
		}


		if (!Character->GetIsInVehicle())
		{
			OwningVehicle->RemovePassenger(i);
		}
		else
		{
			HasRemainingPassengers = true;

			if (!Character->GetIsExitingVehicle())
			{
				if (VehicleSeat->IsSeatLeftSide)
				{
					if (RopeLeft && !RopeLeft->GetIsRopeOccupied())
					{
						RopeLeft->AttachActorToRope(Character);
						Character->SetIsExitingVehicle(true);
					}
				}
				else
				{
					if (RopeRight && !RopeRight->GetIsRopeOccupied())
					{
						RopeRight->AttachActorToRope(Character);
						Character->SetIsExitingVehicle(true);
					}
				}
			}
		}

	}

	if (!HasRemainingPassengers)
	{
		if (RopeLeft) {
			RopeLeft->ReleaseRope();
		}

		if (RopeRight) {
			RopeRight->ReleaseRope();
		}

		// Let the ropes fall to the ground then resume path
		GetOwner()->GetWorldTimerManager().SetTimer(THandler_ResumePath, this, &UVehiclePathFollowerComponent::ResumePath, 1.f, false, 1.f);
		GetOwner()->GetWorldTimerManager().ClearTimer(THandler_ExitPassenger);
	}

}

void UVehiclePathFollowerComponent::SpawnRope()
{
	if (!RopeClass) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwningVehicle;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (RopeLeft == nullptr)
	{
		RopeLeft = GetWorld()->SpawnActor<ARope>(RopeClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (RopeLeft) {
			RopeLeft->AttachToComponent(OwningVehicle->GetMeshComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftRopeSocket);
			RopeLeft->SetRopeLeft(true);
			RopeLeft->DropRope();
		}

	}

	if (RopeRight == nullptr)
	{
		RopeRight = GetWorld()->SpawnActor<ARope>(RopeClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (RopeRight) {
			RopeRight->AttachToComponent(OwningVehicle->GetMeshComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightRopeSocket);
			RopeRight->DropRope();
		}
	}


}

void UVehiclePathFollowerComponent::SetVehicleExit(ABaseCharacter* Character)
{
	auto VehicleSeats = Character->GetVehicletSeat();
}

void UVehiclePathFollowerComponent::SetRopeFree(FVehicletSeating VehicletSeat)
{
	if (!VehicletSeat.OwningVehicle) {
		return;
	}


	auto ActorComponent = VehicletSeat.OwningVehicle->GetComponentByClass(UVehiclePathFollowerComponent::StaticClass());

	if (!ActorComponent) {
		return;
	}

	auto VehiclePathComp = Cast<UVehiclePathFollowerComponent>(ActorComponent);
	if (!VehiclePathComp) {
		return;
	}


	if (VehicletSeat.IsSeatLeftSide)
	{
		VehiclePathComp->GetRopeLeft()->DettachActorToRope();
	}
	else
	{
		VehiclePathComp->GetRopeRight()->DettachActorToRope();
	}
}

void UVehiclePathFollowerComponent::ResumePath()
{
	if (CurveTimeline.IsPlaying()) {
		return;
	}
	
	if (THandler_ExitPassenger.IsValid()) {
		GetOwner()->GetWorldTimerManager().ClearTimer(THandler_ExitPassenger);
	}

	if (THandler_ResumePath.IsValid()) {
		GetOwner()->GetWorldTimerManager().ClearTimer(THandler_ResumePath);
	}

	CurveTimeline.Play();

	PrimaryComponentTick.bCanEverTick = true;
}

void UVehiclePathFollowerComponent::Stop()
{
	if (!CurveTimeline.IsPlaying()) {
		return;
	}

	CurveTimeline.Stop();
	PrimaryComponentTick.bCanEverTick = false;
}

void UVehiclePathFollowerComponent::ResumeNormalSpeed()
{
	CurveTimeline.SetPlayRate(1.0f / PathFollowDuration);
}

void UVehiclePathFollowerComponent::Slowdown()
{
	CurveTimeline.SetPlayRate(1.0f / (PathFollowDuration * 2.f));
}

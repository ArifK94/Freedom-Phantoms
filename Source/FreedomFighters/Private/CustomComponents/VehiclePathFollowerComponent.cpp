// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/VehiclePathFollowerComponent.h"
#include "Components/SplineComponent.h"
#include "Props/VehicleSplinePath.h"

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
	DestroyOnPathComplete = true;

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


	FindPath();
}

void UVehiclePathFollowerComponent::BeginPlay()
{
	Super::BeginPlay();
	GetOwner()->GetWorldTimerManager().SetTimer(THandler_Update, this, &UVehiclePathFollowerComponent::Update, 1.f, true);


	Init();

	// Call after spawning passengers, otherwise character movement component will force characters to be on the ground
	SpawnRandomLocation();
}


void UVehiclePathFollowerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CurveTimeline.TickTimeline(DeltaTime);

}

void UVehiclePathFollowerComponent::Update()
{
	//CurveTimeline.TickTimeline(GetWorld()->TimeSeconds);
}

void UVehiclePathFollowerComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (VehiclePath == nullptr) {
		return;
	}

	FVector CollisionLocation = GetOwner()->GetActorLocation();
	auto CollidedPath = Cast<AVehicleSplinePath>(OtherActor);

	// if collided with another aircraft path then ignore then do not proceed
	if (CollidedPath != VehiclePath) {
		return;
	}

	// find current spline point
	FVehicleSplinePoint CurrentSplinePoint = CollidedPath->GetVehicleSplinePoint(CollisionLocation);

	// if current spline point not found 
	if (CurrentSplinePoint.PointIndex == -1) {
		return;
	}

	// update the aircraft movement type
	CurrentVehicleMovement = CurrentSplinePoint.MovementType;

	// adjust path duration to change speed if specified
	if (CurrentSplinePoint.AffectSpeedType == EVehicleSpeedType::Specified)
	{

		CurveTimeline.SetPlayRate(1.0f / CurrentSplinePoint.PathDuration);
	}
	else
	{
		CurveTimeline.SetPlayRate(1.0f / PathFollowDuration);
	}

	if (CurrentSplinePoint.IsPathFreeToUse)
	{
		VehiclePath->SetOccupantVehicle(nullptr);
	}

	// Wait at current point
	if (CurrentVehicleMovement == EVehicleMovement::Waiting)
	{
		// Stop moving
		CurveTimeline.Stop();

		// Start timer & set the delay based on the duration
		GetOwner()->GetWorldTimerManager().SetTimer(THandler_WaitingMovment, this, &UVehiclePathFollowerComponent::ResumePath, 1.f, false, CurrentSplinePoint.WaitingDuration);
	}
}

void UVehiclePathFollowerComponent::FindPath()
{
	if (FollowTargetDestination)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("MoveToLocation"));
		CurveTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
		CurveTimeline.SetLooping(false);
		CurveTimeline.SetPlayRate(1.0f / PathFollowDuration);
		CurveTimeline.PlayFromStart();
	}
	else
	{
		// if already assigned a path then return
		if (VehiclePath != nullptr)
		{
			return;
		}

		AVehicleSplinePath* ClosestPath = nullptr;
		float ClosestDistance = 0.0f;
		TArray<AActor*> TargetActor;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), VehiclePathTagName, TargetActor);

		for (AActor* Actor : TargetActor)
		{
			auto Path = Cast<AVehicleSplinePath>(Actor);

			// check if path is not occupied
			if (Path && !Path->GetOccupiedVehicle())
			{
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

		if (ClosestPath)
		{
			VehiclePath = ClosestPath;
			VehiclePath->SetOccupantVehicle(GetOwner());

			// setup time line for following the path
			if (CurveFloat)
			{
				FOnTimelineFloat TimelineProgress;
				TimelineProgress.BindUFunction(this, FName("FollowSplinePath"));
				CurveTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
				CurveTimeline.SetLooping(false);
				CurveTimeline.SetPlayRate(1.0f / PathFollowDuration);
				CurveTimeline.PlayFromStart();
			}
		}
	}

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
		if (CurrentLap < TotalLaps) // restart the lap if laps remaining
		{
			CurrentLap++;
			CurveTimeline.PlayFromStart();
		}
		else
		{
			if (DestroyOnPathComplete)
			{
				GetOwner()->Destroy();
			}
		}
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

void UVehiclePathFollowerComponent::ResumePath()
{
	CurveTimeline.Play();
	GetOwner()->GetWorldTimerManager().ClearTimer(THandler_WaitingMovment);
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
		GetOwner()->GetWorldTimerManager().ClearTimer(THandler_WaitingMovment);
	}
}
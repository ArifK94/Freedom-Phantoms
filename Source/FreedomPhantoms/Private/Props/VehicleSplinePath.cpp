// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/VehicleSplinePath.h"

#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

AVehicleSplinePath::AVehicleSplinePath()
{
	PrimaryActorTick.bCanEverTick = false;

	SplinePathComp = CreateDefaultSubobject<USplineComponent>("SplinePathComp");

	OverridePathDurationType = EVehicleSpeedType::Normal;
	OverridePathDuration = 0.f;

	TimeLimit = 0.f;

	HasRandomStartingPoint = false;
	HasInifiteLaps = false;
}

void AVehicleSplinePath::BeginPlay()
{
	Super::BeginPlay();

	UpdatePoints();
}


AVehicleSplinePath* AVehicleSplinePath::FindVehiclePath(UWorld* World, FName TagName)
{
	AVehicleSplinePath* ClosestPath = nullptr;
	float ClosestDistance = 0.0f;
	TArray<AActor*> TargetActor;
	UGameplayStatics::GetAllActorsWithTag(World, TagName, TargetActor);

	for (AActor* Actor : TargetActor)
	{
		auto Path = Cast<AVehicleSplinePath>(Actor);

		// check if path is not occupied
		if (Path && Path->IsPathFree())
		{
			ClosestPath = Path;
		}
	}

	return ClosestPath;
}

int AVehicleSplinePath::GetVehicleSplinePoint(FVector TargetLocation)
{
	for (int i = 0; i < SplinePathComp->GetNumberOfSplinePoints(); i++)
	{
		const FVector StartPoint = SplinePathComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Type::World);

		if (UKismetMathLibrary::EqualEqual_VectorVector(StartPoint, TargetLocation, 500.0f)) {
			return i;
		}
	}
	return -1;
}

FVehicleSplinePoint AVehicleSplinePath::GetNextSplinePoint(int Index)
{
	if (Index < SplinePathComp->GetNumberOfSplinePoints() - 1)
	{
		return VehicleSplinePoints[Index];
	}

	FVehicleSplinePoint SplinePoint = FVehicleSplinePoint();
	SplinePoint.PointIndex = -1;
	return FVehicleSplinePoint();
}

void AVehicleSplinePath::GetFirstSplinePoint(FVector& OutLocation, FRotator& OutRotation)
{
	FVector Loc = FVector::ZeroVector;
	FRotator Rot = FRotator::ZeroRotator;

	Loc = SplinePathComp->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	Rot = SplinePathComp->GetRotationAtSplinePoint(0, ESplineCoordinateSpace::World);

	OutLocation = Loc;
	OutRotation = Rot;
}

bool AVehicleSplinePath::IsPathFree()
{
	return !UKismetSystemLibrary::IsValid(OccupiedVehicle);
}

float AVehicleSplinePath::GetOverridePathDuration(bool& IsOverride)
{
	IsOverride = OverridePathDurationType == EVehicleSpeedType::Specified;
	return IsOverride ? OverridePathDuration : 0.f;
}

void AVehicleSplinePath::StartDurationLimit()
{
	if (TimeLimit > 0.f)
	{
		GetWorldTimerManager().SetTimer(THandler_DurationLimit, this, &AVehicleSplinePath::DestroyVehicle, 1.f, false, TimeLimit);
	}
}

float AVehicleSplinePath::GetStartPointLength()
{
	return HasRandomStartingPoint ? FMath::RandRange(0.f, SplinePathComp->GetSplineLength() - 1) : 0.f;
}

void AVehicleSplinePath::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const int32 TotalSplinePoints = SplinePathComp->GetNumberOfSplinePoints();

	// creating list of aircraft points dynamically
	// start the first index from the list so adding to an existing spline will not duplicate the current items within the list
	for (int SplineCount = VehicleSplinePoints.Num(); SplineCount < TotalSplinePoints; SplineCount++)
	{
		// define the positions of the points and tangents
		const FVector StartPoint = SplinePathComp->GetLocationAtSplinePoint(SplineCount, ESplineCoordinateSpace::Type::World);

		FVehicleSplinePoint VehicleSplinePoint = FVehicleSplinePoint();
		VehicleSplinePoint.PointIndex = SplineCount;
		VehicleSplinePoint.PointLocation = StartPoint;

		VehicleSplinePoints.Add(VehicleSplinePoint);
	}

	UpdatePoints();
}


#if WITH_EDITOR
void AVehicleSplinePath::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdatePoints();
}
#endif


void AVehicleSplinePath::UpdatePoints()
{
	if (VehicleSplinePoints.Num() <= 0) {
		return;
	}

	// setting the collision boxes
	for (int i = 0; i < VehicleSplinePoints.Num(); i++)
	{
		const FVector StartPoint = SplinePathComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Type::World);
		VehicleSplinePoints[i].PointIndex = i;

		UpdatePointerPoints(i);

		if (VehicleSplinePoints[i].ArrowComponent)
		{
			VehicleSplinePoints[i].ArrowComponent->SetWorldLocation(StartPoint);

			if (i + 1 < VehicleSplinePoints.Num())
			{
				const FVector NextPoint = SplinePathComp->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Type::World);
				VehicleSplinePoints[i].ArrowComponent->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(StartPoint, NextPoint));
			}
		}

		if (VehicleSplinePoints[i].TextRenderComponent)
		{
			VehicleSplinePoints[i].TextRenderComponent->SetText(FText::AsNumber(i));
			VehicleSplinePoints[i].TextRenderComponent->SetWorldLocation(StartPoint);
		}


		if (VehicleSplinePoints[i].CollisionBox)
		{
			VehicleSplinePoints[i].CollisionBox->SetWorldLocation(StartPoint);
		}

		if (VehicleSplinePoints[i].CollisionSphere)
		{
			VehicleSplinePoints[i].CollisionSphere->SetWorldLocation(StartPoint);
		}

		FColor ColliderColour;
		switch (VehicleSplinePoints[i].MovementType)
		{
		case EVehicleMovement::MovingForward:
			ColliderColour = FColor(0, 0, 255, 255);
			break;
		case EVehicleMovement::PassengerExit:
			ColliderColour = FColor(0, 255, 0, 255);
			break;
		case EVehicleMovement::Waiting:
			ColliderColour = FColor(255, 0, 255, 255);
			break;
		default:
			ColliderColour = FColor(0, 0, 255, 255);
			break;
		}

		VehicleSplinePoints[i].CollisionBox->ShapeColor = ColliderColour;
		VehicleSplinePoints[i].CollisionSphere->ShapeColor = ColliderColour;
	}

}


void AVehicleSplinePath::UpdatePointerPoints(int Index)
{
	if (VehicleSplinePoints[Index].CollisionBox == nullptr)
	{
		VehicleSplinePoints[Index].CollisionBox = NewObject<UBoxComponent>(this);
		VehicleSplinePoints[Index].CollisionBox->RegisterComponent();
		VehicleSplinePoints[Index].CollisionBox->SetCollisionProfileName(TEXT("OverlapAll"));
	}

	if (VehicleSplinePoints[Index].CollisionSphere == nullptr)
	{
		VehicleSplinePoints[Index].CollisionSphere = NewObject<USphereComponent>(this);
		VehicleSplinePoints[Index].CollisionSphere->RegisterComponent();
		VehicleSplinePoints[Index].CollisionSphere->SetSphereRadius(500.0f);
		VehicleSplinePoints[Index].CollisionSphere->SetCollisionProfileName(TEXT("OverlapAll"));
	}

	if (VehicleSplinePoints[Index].ArrowComponent == nullptr)
	{
		VehicleSplinePoints[Index].ArrowComponent = NewObject<UArrowComponent>(this);
		VehicleSplinePoints[Index].ArrowComponent->RegisterComponent();
	}

	if (VehicleSplinePoints[Index].TextRenderComponent == nullptr)
	{
		VehicleSplinePoints[Index].TextRenderComponent = NewObject<UTextRenderComponent>(this);
		VehicleSplinePoints[Index].TextRenderComponent->RegisterComponent();
		VehicleSplinePoints[Index].TextRenderComponent->SetHiddenInGame(true);
		VehicleSplinePoints[Index].TextRenderComponent->SetWorldSize(100.f);
	}
}

void AVehicleSplinePath::DestroyVehicle()
{
	if (OccupiedVehicle)
	{
		OccupiedVehicle->Destroy();
	}

	GetWorldTimerManager().ClearTimer(THandler_DurationLimit);
}

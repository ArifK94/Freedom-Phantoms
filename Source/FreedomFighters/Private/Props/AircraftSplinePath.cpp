// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/AircraftSplinePath.h"

#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

#include "Kismet/KismetMathLibrary.h"

#include <array>
#include "Containers/Array.h"

AAircraftSplinePath::AAircraftSplinePath()
{
	PrimaryActorTick.bCanEverTick = false;

	SplinePathComp = CreateDefaultSubobject<USplineComponent>("SplinePathComp");
}

FVehicleSplinePoint AAircraftSplinePath::GetVehicleSplinePoint(FVector TargetLocation)
{
	for (int i = 0; i < SplinePathComp->GetNumberOfSplinePoints(); i++)
	{
		const FVector StartPoint = SplinePathComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Type::World);

		if (UKismetMathLibrary::EqualEqual_VectorVector(StartPoint, TargetLocation, 100.0f)) {
			return VehicleSplinePoints[i];
		}
	}

	FVehicleSplinePoint SplinePoint = FVehicleSplinePoint();
	SplinePoint.PointIndex = -1;
	return SplinePoint;
}

FVehicleSplinePoint AAircraftSplinePath::GetNextSplinePoint(int Index)
{
	if (Index < SplinePathComp->GetNumberOfSplinePoints() - 1)
	{
		return VehicleSplinePoints[Index];
	}

	FVehicleSplinePoint SplinePoint = FVehicleSplinePoint();
	SplinePoint.PointIndex = -1;
	return FVehicleSplinePoint();
}

void AAircraftSplinePath::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const int32 TotalSplinePoints = SplinePathComp->GetNumberOfSplinePoints();

	// creating list of aircraft points dynamically
	for (int SplineCount = VehicleSplinePoints.Num(); SplineCount < TotalSplinePoints; SplineCount++)
	{
		// define the positions of the points and tangents
		const FVector StartPoint = SplinePathComp->GetLocationAtSplinePoint(SplineCount, ESplineCoordinateSpace::Type::World);

		FVehicleSplinePoint VehicleSplinePoint = FVehicleSplinePoint();
		VehicleSplinePoint.PointIndex = SplineCount;
		VehicleSplinePoint.PointLocation = StartPoint;

		VehicleSplinePoints.Add(VehicleSplinePoint);
	}



	UpdateCollisionBox();
	UpdatePointIndex();

	//updating the aircraft points if any are removed from the editor
	//if (VehicleSplinePoints.Num() > TotalSplinePoints)
	//{
	//	const int32 UnusedPoints = VehicleSplinePoints.Num() - TotalSplinePoints;
	//	for (int i = VehicleSplinePoints.Num(); i > UnusedPoints; i++)
	//	{
	//		//VehicleSplinePoints.RemoveAt(i);
	//	}
	//}

}

void AAircraftSplinePath::PostEditChangeProperty(FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);

	UpdateCollisionBox();
	UpdatePointIndex();
}

void AAircraftSplinePath::UpdatePointIndex()
{
	for (int i = 0; i < SplinePathComp->GetNumberOfSplinePoints(); i++)
	{
		for (int j = 0; j < VehicleSplinePoints.Num(); j++)
		{
			FVehicleSplinePoint VehicleSplinePoint = VehicleSplinePoints[j];
			VehicleSplinePoint.PointIndex = i;
		}
	}
}

void AAircraftSplinePath::UpdateCollisionBox()
{
	// setting the collision boxes
	for (int i = 0; i < SplinePathComp->GetNumberOfSplinePoints(); i++)
	{
		const FVector StartPoint = SplinePathComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Type::World);

		UBoxComponent* CollisionBox = NewObject<UBoxComponent>(this, UBoxComponent::StaticClass());
		CollisionBox->SetWorldLocation(StartPoint);

		USphereComponent* CollisionSphere = NewObject<USphereComponent>(this, USphereComponent::StaticClass());
		CollisionSphere->InitSphereRadius(500.0f);
		CollisionSphere->SetWorldLocation(StartPoint);

		FVehicleSplinePoint VehicleSplinePoint = VehicleSplinePoints[i];
		
		switch (VehicleSplinePoint.MovementType)
		{
		case AircraftSplineMovement::Throttling:
			CollisionBox->ShapeColor = FColor(0, 0, 255, 255);
			break;
		case AircraftSplineMovement::Hovering:
			CollisionBox->ShapeColor = FColor(0, 255, 0, 255);
			break;
		default:
			CollisionBox->ShapeColor = FColor(0, 0, 255, 255);
			break;
		}

	}

}


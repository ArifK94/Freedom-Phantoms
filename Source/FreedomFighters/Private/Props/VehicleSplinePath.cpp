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
}

void AVehicleSplinePath::BeginPlay()
{
	Super::BeginPlay();

	UpdateCollisionBox();
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
		if (Path && !Path->GetOccupiedVehicle())
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

void AVehicleSplinePath::GetFirstSplinePoint(FVector Location, FRotator Rotation)
{
	FVector Loc = FVector::ZeroVector;
	FRotator Rot = FRotator::ZeroRotator;

	Loc = SplinePathComp->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	Rot = SplinePathComp->GetRotationAtSplinePoint(0, ESplineCoordinateSpace::World);

	Location = Loc;
	Rotation = Rot;
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


#if WITH_EDITOR
void AVehicleSplinePath::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdateCollisionBox();
	UpdatePointIndex();
}
#endif


void AVehicleSplinePath::UpdatePointIndex()
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

void AVehicleSplinePath::UpdateCollisionBox()
{
	if (SplinePathComp->GetNumberOfSplinePoints() <= 0) {
		return;
	}

	// setting the collision boxes
	for (int i = 0; i < SplinePathComp->GetNumberOfSplinePoints(); i++)
	{
		const FVector StartPoint = SplinePathComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Type::World);
		FVehicleSplinePoint VehicleSplinePoint = VehicleSplinePoints[i];

		UpdatePointerPoints();

		if (VehicleSplinePointsPtr[i]->ArrowComponent)
		{
			VehicleSplinePointsPtr[i]->ArrowComponent->SetWorldLocation(StartPoint);
		}

		if (VehicleSplinePointsPtr[i]->TextRenderComponent)
		{
			VehicleSplinePointsPtr[i]->TextRenderComponent->SetText(FText::AsNumber(i));
			VehicleSplinePointsPtr[i]->TextRenderComponent->SetWorldLocation(StartPoint);
		}


		if (VehicleSplinePointsPtr[i]->CollisionBox)
		{
			VehicleSplinePointsPtr[i]->CollisionBox->SetWorldLocation(StartPoint);
		}

		if (VehicleSplinePointsPtr[i]->CollisionSphere)
		{
			VehicleSplinePointsPtr[i]->CollisionSphere->SetWorldLocation(StartPoint);
		}

		FColor ColliderColour;
		switch (VehicleSplinePoint.MovementType)
		{
		case EVehicleMovement::MovingForward:
			ColliderColour = FColor(0, 0, 255, 255);
			break;
		case EVehicleMovement::Rappel:
			ColliderColour = FColor(0, 255, 0, 255);
			break;
		default:
			ColliderColour = FColor(0, 0, 255, 255);
			break;
		}

		VehicleSplinePointsPtr[i]->CollisionBox->ShapeColor = ColliderColour;
		VehicleSplinePointsPtr[i]->CollisionSphere->ShapeColor = ColliderColour;
	}

}


void AVehicleSplinePath::UpdatePointerPoints()
{
	auto VehicleSplinePointPtr = new FVehicleSplinePoint();

	if (VehicleSplinePointPtr->CollisionBox == nullptr)
	{
		VehicleSplinePointPtr->CollisionBox = NewObject<UBoxComponent>(this);
		VehicleSplinePointPtr->CollisionBox->RegisterComponent();
		VehicleSplinePointPtr->CollisionBox->SetCollisionProfileName(TEXT("OverlapAll"));
	}

	if (VehicleSplinePointPtr->CollisionSphere == nullptr)
	{
		VehicleSplinePointPtr->CollisionSphere = NewObject<USphereComponent>(this);
		VehicleSplinePointPtr->CollisionSphere->RegisterComponent();
		VehicleSplinePointPtr->CollisionSphere->SetSphereRadius(500.0f);
		VehicleSplinePointPtr->CollisionSphere->SetCollisionProfileName(TEXT("OverlapAll"));
	}

	if (VehicleSplinePointPtr->ArrowComponent == nullptr)
	{
		VehicleSplinePointPtr->ArrowComponent = NewObject<UArrowComponent>(this);
		VehicleSplinePointPtr->ArrowComponent->RegisterComponent();
	}

	if (VehicleSplinePointPtr->TextRenderComponent == nullptr)
	{
		VehicleSplinePointPtr->TextRenderComponent = NewObject<UTextRenderComponent>(this);
		VehicleSplinePointPtr->TextRenderComponent->RegisterComponent();
		VehicleSplinePointPtr->TextRenderComponent->SetHiddenInGame(true);
		VehicleSplinePointPtr->TextRenderComponent->SetWorldSize(100.f);
	}

	if (!VehicleSplinePointsPtr.Contains(VehicleSplinePointPtr))
	{
		VehicleSplinePointsPtr.Add(VehicleSplinePointPtr);
	}

}
// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/PatrolFollowerComponent.h"
#include "Props/PatrolPath.h"

#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"

UPatrolFollowerComponent::UPatrolFollowerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CurrentPathIndex = 0;

	IsPatrolASC = true;
}


void UPatrolFollowerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PatrolPath) {
		SplinePathComp = PatrolPath->GetSplinePathComp();
	}
}

FVector UPatrolFollowerComponent::GetCurrentPathPoint()
{
	auto OutLocation = FVector::ZeroVector;

	if (!SplinePathComp) {
		if (PatrolPath) {
			SplinePathComp = PatrolPath->GetSplinePathComp();
		}
		else {
			return OutLocation;
		}
	}

	if (!SplinePathComp) {
		return OutLocation;
	}

	return SplinePathComp->GetLocationAtSplinePoint(CurrentPathIndex, ESplineCoordinateSpace::World);
}


FVector UPatrolFollowerComponent::GetNextPathPoint()
{
	auto OutLocation = FVector::ZeroVector;

	if (!SplinePathComp) {
		if (PatrolPath) {
			SplinePathComp = PatrolPath->GetSplinePathComp();
		}
		else {
			return OutLocation;
		}
	}

	if (!SplinePathComp) {
		return OutLocation;
	}

	int TotalPoints = SplinePathComp->GetNumberOfSplinePoints();
	int NextPoint = CurrentPathIndex + 1; // Predict the next point

	if (!IsPatrolASC)
	{
		NextPoint = CurrentPathIndex - 1;
	}

	bool HasReachedStart = NextPoint < 0; // Returned to starting point
	bool HasReachedEnd = NextPoint >= TotalPoints;

	
	if (HasReachedStart)
	{
		IsPatrolASC = true;
	}
	else if (HasReachedEnd)
	{
		IsPatrolASC = false;
	}

	if (IsPatrolASC)
	{
		CurrentPathIndex++;
	}
	else
	{
		CurrentPathIndex--;
	}

	return SplinePathComp->GetLocationAtSplinePoint(CurrentPathIndex, ESplineCoordinateSpace::World);
}
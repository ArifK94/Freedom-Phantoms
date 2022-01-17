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

	//if (!PatrolPath)
	//{
	//	float ClosestDistance = 0.0f;
	//	TArray<AActor*> TargetActor;
	//	UGameplayStatics::GetAllActorsWithTag(GetWorld(), PathTagName, TargetActor);
	//}

	if (PatrolPath) {
		SplinePathComp = PatrolPath->GetSplinePathComp();
	}
}

void UPatrolFollowerComponent::GetCurrentPathPoint(FVector& OutLocation)
{
	OutLocation = FVector::ZeroVector;

	if (!SplinePathComp) {
		if (PatrolPath) {
			SplinePathComp = PatrolPath->GetSplinePathComp();
		}
		else {
			return;
		}
	}

	if (!SplinePathComp) {
		return;
	}

	OutLocation = SplinePathComp->GetLocationAtSplinePoint(CurrentPathIndex, ESplineCoordinateSpace::World);
}


void UPatrolFollowerComponent::GetNextPathPoint(FVector& OutLocation)
{
	OutLocation = FVector::ZeroVector;

	if (!SplinePathComp) {
		if (PatrolPath) {
			SplinePathComp = PatrolPath->GetSplinePathComp();
		}
		else {
			return;
		}
	}

	if (!SplinePathComp) {
		return;
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

	OutLocation = SplinePathComp->GetLocationAtSplinePoint(CurrentPathIndex, ESplineCoordinateSpace::World);
}
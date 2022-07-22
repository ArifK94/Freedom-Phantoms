// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/CoverFinderComponent.h"
#include "Managers/GameModeManager.h"
#include "Services/SharedService.h"
#include "FreedomFighters/FreedomFighters.h"

#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

UCoverFinderComponent::UCoverFinderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CoverRadius = 10.f;
	CoverLength = 1000.f;

	NumberOfCoverTraces = 35;
}

void UCoverFinderComponent::BeginPlay()
{
	Super::BeginPlay();

	GameModeManager = Cast<AGameModeManager>(GetWorld()->GetAuthGameMode());
	Controller = Cast<AController>(GetOwner());
	Pawn = Controller->GetPawn();

	//CoverSphere = NewObject<USphereComponent>(GetOwner());
	//CoverSphere->RegisterComponent();
	//CoverSphere->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	//CoverSphere->SetSphereRadius(CoverRadius);
	//CoverSphere->SetCollisionProfileName(TEXT("OverlapAll"));
}


bool UCoverFinderComponent::FindCover(FVector StartLocation, FVector& ChosenCoverPoint)
{
	ChosenCoverPoint = FVector::ZeroVector;

	if (StartLocation.IsZero()) {
		return false;
	}

	TArray<FVector> CoverLocationPoints;

	float factor = 360.f / NumberOfCoverTraces;

	// 360 degrees line trace around character
	for (int i = 0; i < NumberOfCoverTraces; i++)
	{
		FNavLocation NavLocation;
		bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(StartLocation, NavLocation);

		if (!bOnNavMesh) {
			continue;
		}

		const FQuat CharacterTopRotation = UKismetMathLibrary::MakeRotator(0.0f, 0.0f, i * factor).Quaternion();
		FVector Foward = UKismetMathLibrary::Quat_RotateVector(CharacterTopRotation, FVector(1.0f, 0.0f, 0.0f));

		FVector Start = NavLocation.Location;
		FVector End = (Foward * CoverLength) + Start;

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, COLLISION_COVER);

		//DrawDebugLine(GetWorld(), Start, End, FColor::Cyan, true, 5.0f, 0, 1.0f);

		// get all hit results which hit an obstacle
		if (!bHit) {
			continue;
		}

		// add 50.f to to allow the destination to be reachable on the navmesh 
		FVector LocationPoint = HitResult.ImpactPoint + 50.f;

		DrawDebugSphere(GetWorld(), LocationPoint, 5.f, 10.f, FColor::Red, false, 10.f, 0, 2);

		if (LocationPoint.IsZero()) {
			continue;
		}

		// has this location already been added to the list?
		if (CoverLocationPoints.Contains(LocationPoint)) {
			continue;
		}

		// is the cover point taken by someone else?
		if (IsCoverPointTaken(LocationPoint)) {
			continue;
		}

		CoverLocationPoints.Add(LocationPoint);
	}


	if (CoverLocationPoints.Num() > 0)
	{
		auto PointLocation = GetClosestCoverPoint(CoverLocationPoints);

		if (!PointLocation.IsZero())
		{
			ChosenCoverPoint = PointLocation;
		}
	}

	return !ChosenCoverPoint.IsZero();
}

bool UCoverFinderComponent::FindCover(AActor* TargetActor, FVector& ChosenCoverPoint)
{
	ChosenCoverPoint = FVector::ZeroVector;

	if (TargetActor == nullptr || TargetActor->GetActorLocation().IsZero()) {
		return false;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Pawn);
	QueryParams.bTraceComplex = true;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;

	TArray<FVector> CoverLocationPoints;

	float factor = 360.f / NumberOfCoverTraces;

	// 360 degrees line trace around character
	for (int i = 0; i < NumberOfCoverTraces; i++)
	{
		FNavLocation NavLocation;
		bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(Pawn->GetActorLocation(), NavLocation);

		if (!bOnNavMesh) {
			continue;
		}

		const FQuat CharacterTopRotation = UKismetMathLibrary::MakeRotator(0.0f, 0.0f, i * factor).Quaternion();
		FVector Foward = UKismetMathLibrary::Quat_RotateVector(CharacterTopRotation, FVector(1.0f, 0.0f, 0.0f));

		FVector Start = NavLocation.Location;
		FVector End = (Foward * CoverLength) + Start;

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, COLLISION_COVER);


		// get all hit results which hit an obstacle
		if (!bHit) {
			continue;
		}

		// avoid hitpoints at the origin.
		if (HitResult.ImpactPoint.IsZero()) {
			continue;
		}


		// the gap between impact point & the location on the navmesh for the npc to move to.
		float CoverOffset = 50.f;

		// based on which side the cover point is of the object, the offset will need to be negative or positive.
		auto DistanceX = HitResult.ImpactNormal.X > 0 ? CoverOffset : -CoverOffset;
		auto DistanceY = HitResult.ImpactNormal.Y > 0 ? CoverOffset : -CoverOffset;


		// add offset to to allow the destination to be reachable on the navmesh 
		FVector LocationPoint = FVector(
			HitResult.ImpactPoint.X + DistanceX,
			HitResult.ImpactPoint.Y + DistanceY,
			HitResult.ImpactPoint.Z
		);

		// we want cover points which the target actor cannot see while taking cover.
		if (SharedService::CanSeeTarget(GetWorld(), LocationPoint, TargetActor, Pawn)) {
			continue;
		}


		// check if offset to cover points can see the target, if not, then ignore this cover point.
		// This is to allow the character to peak out & shoot the enemy while in cover.
		float Offset = 150.f;
		FVector Left = FVector(LocationPoint.X, LocationPoint.Y - Offset, LocationPoint.Z);
		FVector Right = FVector(LocationPoint.X, LocationPoint.Y + Offset, LocationPoint.Z);
		FVector Top = FVector(LocationPoint.X, LocationPoint.Y, LocationPoint.Z + Offset);


		bool CanSeeTarget =
			SharedService::CanSeeTarget(GetWorld(), Left, TargetActor, Pawn) ||
			SharedService::CanSeeTarget(GetWorld(), Right, TargetActor, Pawn) ||
			SharedService::CanSeeTarget(GetWorld(), Top, TargetActor, Pawn);


		// if no line traces can see the target actor, then ignore this cover point.
		if (!CanSeeTarget) {
			continue;
		}


		// has this location already been added to the list?
		if (CoverLocationPoints.Contains(LocationPoint)) {
			continue;
		}

		// is the cover point taken by someone else?
		if (IsCoverPointTaken(LocationPoint)) {
			continue;
		}

		CoverLocationPoints.Add(LocationPoint);
	}

	if (CoverLocationPoints.Num() > 0)
	{
		auto PointLocation = GetClosestCoverPoint(CoverLocationPoints);

		if (!PointLocation.IsZero())
		{
			ChosenCoverPoint = PointLocation;
		}
	}

	return !ChosenCoverPoint.IsZero();
}

FVector UCoverFinderComponent::GetClosestCoverPoint(TArray<FVector> CoverLocationPoints)
{
	FVector ClosestPoint = FVector::ZeroVector;
	float minDist = CoverRadius;

	for (int i = 0; i < CoverLocationPoints.Num(); i++)
	{
		FVector Point = CoverLocationPoints[i];

		float Distance = FVector::Dist(Pawn->GetActorLocation(), Point);

		if (Distance < minDist || ClosestPoint.IsZero())
		{
			ClosestPoint = Point;
			minDist = Distance;
		}
	}

	// ignore origin location
	if (ClosestPoint.IsZero()) {
		return ClosestPoint;
	}

	// update current cover point, 
	FWorldCoverPoint CoverLocation = FWorldCoverPoint();
	CoverLocation.Owner = Pawn;

	// remove previous cover point from the list
	CoverLocation.Location = ClosestPoint;
	GameModeManager->RemoveCoverPoint(CoverLocation);

	// add the new cover point
	CoverLocation.Location = ClosestPoint;
	GameModeManager->AddCoverPoint(CoverLocation);

	return ClosestPoint;
}

bool UCoverFinderComponent::IsCoverPointTaken(FVector PointLocation)
{

	// check if current cover has been taken,
	// if so, then find another cover point
	FWorldCoverPoint CoverLocation = FWorldCoverPoint();
	CoverLocation.Location = PointLocation;
	CoverLocation.Owner = Pawn;

	return GameModeManager->IsCoverPointTaken(CoverLocation);
}
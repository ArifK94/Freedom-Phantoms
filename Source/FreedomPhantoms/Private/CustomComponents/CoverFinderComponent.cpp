// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/CoverFinderComponent.h"
#include "Managers/GameModeManager.h"
#include "Services/SharedService.h"
#include "Characters/BaseCharacter.h"
#include "FreedomPhantoms/FreedomPhantoms.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

UCoverFinderComponent::UCoverFinderComponent()
{
	CoverRadius = 1000.f;
	CoverDistance = 150.0f;

	NumberOfCoverTraces = 35;
}

void UCoverFinderComponent::BeginPlay()
{
	Super::BeginPlay();

	DefaultSearchRadius = CoverRadius;
}

void UCoverFinderComponent::TimerTick()
{
	Super::TimerTick();

	Init();
}

void UCoverFinderComponent::Init()
{
	if (!GameModeManager) {
		GameModeManager = Cast<AGameModeManager>(GetWorld()->GetAuthGameMode());
	}

	if (!Controller) {
		Controller = Cast<AController>(GetOwner());
	}

	if (Controller) {
		if (!Character) {
			Character = Cast<ABaseCharacter>(Controller->GetPawn());
		}
	}
}


bool UCoverFinderComponent::FindCover(FVector StartLocation, FTransform& ChosenCoverPoint)
{
	ChosenCoverPoint.SetLocation(FVector::ZeroVector);

	if (StartLocation.IsZero()) {
		return false;
	}

	TArray<FTransform> CoverPoints = GetCoverPoints();

	if (CoverPoints.Num() > 0)
	{
		auto PointLocation = GetClosestCoverPoint(CoverPoints);
		ChosenCoverPoint = PointLocation;
	}

	FCoverSearchParameters CoverSearchParameters;
	CoverSearchParameters.CvoerPoint = ChosenCoverPoint;
	CoverSearchParameters.IsCoverFound = !ChosenCoverPoint.GetLocation().IsZero();
	OnCoverSearch.Broadcast(CoverSearchParameters);

	return !ChosenCoverPoint.GetLocation().IsZero();
}

bool UCoverFinderComponent::FindCover(AActor* TargetActor, FTransform& ChosenCoverPoint)
{
	ChosenCoverPoint.SetLocation(FVector::ZeroVector);

	if (TargetActor == nullptr || TargetActor->GetActorLocation().IsZero()) {
		return false;
	}

	TArray<FTransform> CoverPoints = TArray<FTransform>();

	for (FTransform CoverPoint : GetCoverPoints())
	{
		FVector LocationPoint = CoverPoint.GetLocation();

		// we want cover points which the target actor cannot see while taking cover.
		if (USharedService::CanSeeTarget(GetWorld(), LocationPoint, TargetActor, Character)) {
			continue;
		}

		// check if offset to cover points can see the target, if not, then ignore this cover point.
		// This is to allow the character to peak out & shoot the enemy while in cover.
		float Offset = 150.f;
		FVector Left = FVector(LocationPoint.X, LocationPoint.Y - Offset, LocationPoint.Z);
		FVector Right = FVector(LocationPoint.X, LocationPoint.Y + Offset, LocationPoint.Z);
		FVector Top = FVector(LocationPoint.X, LocationPoint.Y, LocationPoint.Z + Offset);


		bool CanSeeTarget =
			USharedService::CanSeeTarget(GetWorld(), Left, TargetActor, Character) ||
			USharedService::CanSeeTarget(GetWorld(), Right, TargetActor, Character) ||
			USharedService::CanSeeTarget(GetWorld(), Top, TargetActor, Character);


		// if no line traces can see the target actor, then ignore this cover point.
		if (!CanSeeTarget) {
			continue;
		}

		CoverPoints.Add(CoverPoint);
	}

	if (CoverPoints.Num() > 0)
	{
		auto PointLocation = GetClosestCoverPoint(CoverPoints);
		ChosenCoverPoint = PointLocation;
	}

	FCoverSearchParameters CoverSearchParameters;
	CoverSearchParameters.CvoerPoint = ChosenCoverPoint;
	CoverSearchParameters.IsCoverFound = !ChosenCoverPoint.GetLocation().IsZero();
	OnCoverSearch.Broadcast(CoverSearchParameters);

	return !ChosenCoverPoint.GetLocation().IsZero();
}

TArray<FTransform> UCoverFinderComponent::GetCoverPoints()
{
	Init();

	TArray<FTransform> CoverPoints = TArray<FTransform>();

	float factor = 360.f / NumberOfCoverTraces;

	// 360 degrees line trace around character
	for (int i = 0; i < NumberOfCoverTraces; i++)
	{
		FNavLocation NavLocation;
		bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(Character->GetActorLocation(), NavLocation);

		if (!bOnNavMesh) {
			continue;
		}

		const FQuat CharacterTopRotation = UKismetMathLibrary::MakeRotator(0.0f, 0.0f, i * factor).Quaternion();
		FVector Foward = UKismetMathLibrary::Quat_RotateVector(CharacterTopRotation, FVector(1.0f, 0.0f, 0.0f));

		FVector Start = NavLocation.Location;
		FVector End = (Foward * CoverRadius) + Start;

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, COLLISION_COVER);

		// get all hit results which hit an obstacle
		if (!bHit) {
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

		//DrawDebugSphere(GetWorld(), LocationPoint, 5.f, 10.f, FColor::Red, false, 10.f, 0, 2);

		if (LocationPoint.IsZero()) {
			continue;
		}

		// is the cover point taken by someone else?
		if (IsCoverPointTaken(LocationPoint)) {
			continue;
		}

		if (!IsPreferredCover(HitResult.Normal, LocationPoint)) {
			continue;
		}

		FTransform CoverPoint;
		CoverPoint.SetLocation(LocationPoint);
		CoverPoint.SetRotation(UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal).Quaternion());
		CoverPoints.Add(CoverPoint);
	}


	return CoverPoints;
}

FTransform UCoverFinderComponent::GetClosestCoverPoint(TArray<FTransform> CoverLocationPoints)
{
	FTransform ClosestPoint;
	FVector ClosestLocation = FVector::ZeroVector;
	ClosestPoint.SetLocation(ClosestLocation);

	float minDist = CoverRadius;

	Init();

	for (FTransform CoverPoint : CoverLocationPoints)
	{
		FVector Point = CoverPoint.GetLocation();

		float Distance = FVector::Dist(Character->GetActorLocation(), Point);

		if (Distance < minDist || ClosestLocation.IsZero())
		{
			ClosestLocation = Point;
			minDist = Distance;

			ClosestPoint.SetLocation(ClosestLocation);
			CoverPoint.SetRotation(CoverPoint.GetRotation());
		}
	}

	// ignore origin location
	if (ClosestLocation.IsZero()) {
		return ClosestPoint;
	}

	// update current cover point, 
	FWorldCoverPoint CoverLocation = FWorldCoverPoint();
	CoverLocation.Owner = Character;

	RemoveCoverPoint();

	// add the new cover point
	CoverLocation.Location = ClosestLocation;
	GameModeManager->AddCoverPoint(CoverLocation);

	//DrawDebugSphere(GetWorld(), ClosestLocation, 5, 1, FColor::Red, false, 2, 0, 2);


	MyChosenCoverPoint = ClosestLocation;
	return ClosestPoint;
}

void UCoverFinderComponent::GetCorners(FVector WallNormal, FVector CoverLocation, bool& LineTraceLeft, bool& LineTraceRight)
{
	FVector WallDirection = WallNormal * -1.0f; // get direction towards the cover wall
	FVector StartLocation = CoverLocation;

	FVector RightVector = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(WallDirection)) * Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 50.f;
	FVector StartRight = StartLocation + RightVector;
	FVector EndRight = WallDirection * CoverDistance + StartRight;
	FHitResult OutHitRight;
	LineTraceRight = GetWorld()->LineTraceSingleByChannel(OutHitRight, StartRight, EndRight, COLLISION_COVER);

	FVector LeftVector = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(WallNormal)) * Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector StartLeft = StartLocation + LeftVector;
	FVector EndLeft = WallDirection * CoverDistance + StartLeft;
	FHitResult OutHitLeft;
	LineTraceLeft = GetWorld()->LineTraceSingleByChannel(OutHitLeft, StartLeft, EndLeft, COLLISION_COVER);

	// try line tracing for crouching corners if not found line traces for stand state.
	StartLocation.Z -= 50.f;

	if (!LineTraceRight)
	{
		StartRight = StartLocation + RightVector;
		EndRight = WallDirection * CoverDistance + StartRight;
		FHitResult OutHitBelow;
		LineTraceRight = GetWorld()->LineTraceSingleByChannel(OutHitBelow, StartRight, EndRight, COLLISION_COVER);
	}

	if (!LineTraceLeft)
	{
		StartLeft = StartLocation + LeftVector;
		EndLeft = WallDirection * CoverDistance + StartLeft;
		FHitResult OutHitBelow;
		LineTraceLeft = GetWorld()->LineTraceSingleByChannel(OutHitBelow, StartLeft, EndRight, COLLISION_COVER);
	}
}

bool UCoverFinderComponent::CanCoverPeakUp(FVector WallNormal, FVector CoverLocation)
{
	FVector WallDirection = WallNormal * -1.0f; // get direction towards the cover wall

	FVector RightVector = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(WallDirection)) * Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector Start = FVector(CoverLocation.X, CoverLocation.Y, CoverLocation.Z + 100.f + Character->GetCapsuleComponent()->GetScaledCapsuleRadius());
	FVector End = WallDirection * CoverDistance + Start;

	FHitResult OutHit;
	bool LineTrace = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Blue, true, 10, 0, 1);
	//DrawDebugSphere(GetWorld(), Start, 20, 1, FColor::Blue, false, 10, 0, 2);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, true, 999, 0, 1);

	return !LineTrace;
}

bool UCoverFinderComponent::IsPreferredCover(FVector WallNormal, FVector CoverLocation)
{
	bool LineTraceLeft = false;
	bool LineTraceRight = false;
	GetCorners(WallNormal, CoverLocation, LineTraceLeft, LineTraceRight);

	auto CanPeakUp = CanCoverPeakUp(WallNormal, CoverLocation);

	// if has left & right in cover direction & cannot peak up then this cover will not work.
	if (LineTraceLeft && LineTraceRight && !CanPeakUp)
	{
		return false;
	}

	return CanPeakUp || !LineTraceLeft || !LineTraceRight;
}

void UCoverFinderComponent::RemoveCoverPoint()
{
	if (!GameModeManager) {
		return;
	}

	FWorldCoverPoint CoverLocation = FWorldCoverPoint();
	CoverLocation.Owner = Character;

	// remove previous cover point from the list
	CoverLocation.Location = MyChosenCoverPoint;
	GameModeManager->RemoveCoverPoint(CoverLocation);
}

bool UCoverFinderComponent::IsCoverPointTaken(FVector PointLocation)
{
	Init();

	// check if current cover has been taken,
	// if so, then find another cover point
	FWorldCoverPoint CoverLocation = FWorldCoverPoint();
	CoverLocation.Location = PointLocation;
	CoverLocation.Owner = Character;

	return GameModeManager->IsCoverPointTaken(CoverLocation);
}

void UCoverFinderComponent::ResetSearchRadius()
{
	CoverRadius = DefaultSearchRadius;
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/CoverFinderComponent.h"
#include "Managers/GameModeManager.h"

#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

UCoverFinderComponent::UCoverFinderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CoverRadius = 500.f;

	NumberOfCoverTraces = 35;
}

void UCoverFinderComponent::BeginPlay()
{
	Super::BeginPlay();

	//CoverSphere = NewObject<USphereComponent>(GetOwner());
	//CoverSphere->RegisterComponent();
	//CoverSphere->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	//CoverSphere->SetSphereRadius(CoverRadius);
	//CoverSphere->SetCollisionProfileName(TEXT("OverlapAll"));


	GameModeManager = Cast<AGameModeManager>(GetWorld()->GetAuthGameMode());
}


FVector UCoverFinderComponent::FindCover(FVector StartLocation)
{
	auto ChosenCoverPoint = FVector();

	if (StartLocation.IsZero()) {
		return ChosenCoverPoint;
	}

	TArray<FVector> CoverLocationPoints;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bTraceComplex = true;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;

	float factor = 360.0f / NumberOfCoverTraces;

	// 360 degrees line trace around character
	for (int i = 0; i < NumberOfCoverTraces; i++)
	{

		FNavLocation NavLocation;
		bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(GetOwner()->GetActorLocation(), NavLocation);

		if (bOnNavMesh)
		{
			const FQuat CharacterTopRotation = UKismetMathLibrary::MakeRotator(0.0f, 0.0f, i * factor).Quaternion();
			FVector Foward = UKismetMathLibrary::Quat_RotateVector(CharacterTopRotation, FVector(1.0f, 0.0f, 0.0f));

			FVector Start = (Foward * CoverRadius) + NavLocation.Location;

			FHitResult HitResult;
			bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, NavLocation.Location, ECC_Visibility);


			//DrawDebugLine(GetWorld(), Start, NavLocation.Location, FColor::Emerald, true, 5.0f, 0, 1.0f);

			// get all hit results which hit an obstacle
			if (bHit)
			{
				//FVector LocationPoint = HitResult.ImpactPoint + (HitResult.ImpactNormal * 50.0f) + FVector(0.0f, 0.0f, 100.0f);
				FVector LocationPoint = HitResult.ImpactPoint;

				DrawDebugLine(GetWorld(), Start, LocationPoint, FColor::Magenta, true, 50.0f, 0, 1.0f);

				if (!IsCoverPointTaken(LocationPoint))
				{

					bool CanSeeTarget = false;
					float directionValue = FVector::DotProduct(LocationPoint, GetOwner()->GetActorLocation());

					float Offset = 50.0f;
					FHitResult HitTargetResult2, HitTargetResult3;
					bool bTargetHit2 = GetWorld()->LineTraceSingleByObjectType(HitTargetResult2, LocationPoint + FVector(0.0f, Offset, 0.0f), GetOwner()->GetActorLocation(), ObjectParams, QueryParams);
					bool bTargetHit3 = GetWorld()->LineTraceSingleByObjectType(HitTargetResult3, LocationPoint + FVector(0.0f, 0.0f, Offset), GetOwner()->GetActorLocation(), ObjectParams, QueryParams);

					//if (bTargetHit2)
					//{
					//	if (Cast<ACombatCharacter>(HitTargetResult2.GetActor()))
					//	{
					//		CanSeeTarget = true;
					//	}
					//}

					//if (bTargetHit3)
					//{
					//	if (Cast<ACombatCharacter>(HitTargetResult3.GetActor()))
					//	{
					//		CanSeeTarget = true;
					//	}
					//}

					CoverLocationPoints.Add(LocationPoint);

				}

			}
		}
	}





	// create a collision sphere
	//FCollisionShape MyColSphere = FCollisionShape::MakeSphere(CoverRadius);

	//// create tarray for hit results
	//TArray<FHitResult> OutHits;

	//// check if something got hit in the sweep
	//bool isHit = GetWorld()->SweepMultiByChannel(OutHits, GetOwner()->GetActorLocation(), StartLocation, FQuat::Identity, ECC_Visibility, MyColSphere);


	//if (isHit)
	//{
	//	for (auto& Hit : OutHits)
	//	{
	//		//DrawDebugSphere(GetWorld(), Hit.ImpactPoint, CoverRadius, 20, FColor::Purple, false, 100.f, 0, 2);

	//		AActor* DamagedActor = Hit.GetActor();

	//		if (!DamagedActor)
	//		{
	//			FNavLocation NavLocation;
	//			UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);

	//			bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(Hit.ImpactPoint, NavLocation);

	//			if (bOnNavMesh)
	//			{
	//				CoverLocationPoints.Add(NavLocation.Location);
	//			}
	//		}
	//	}
	//}


	if (CoverLocationPoints.Num() > 0)
	{
		auto PointLocation = GetClosestCoverPoint(CoverLocationPoints);

		if (!PointLocation.IsZero())
		{
			ChosenCoverPoint = PointLocation;

			DrawDebugSphere(GetWorld(), ChosenCoverPoint, 80.f, 20, FColor::Purple, false, 100.f, 0, 2);
		}
	}

	return ChosenCoverPoint;
}

FVector UCoverFinderComponent::GetClosestCoverPoint(TArray<FVector> CoverLocationPoints)
{
	FVector ClosestPoint;
	float minDist = CoverRadius;

	for (int i = 0; i < CoverLocationPoints.Num(); i++)
	{
		FVector Point = CoverLocationPoints[i];

		float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Point);

		if (Distance < minDist)
		{
			ClosestPoint = Point;
			minDist = Distance;
		}
	}

	// update current cover point, 
	FWorldCoverPoint CoverLocation = FWorldCoverPoint();
	CoverLocation.Owner = GetOwner();

	// remove previous cover point from the list
	//CoverLocation.Location = ChosenCoverPoint;
	//GameModeManager->RemoveCoverPoint(CoverLocation);

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
	CoverLocation.Owner = GetOwner();

	return GameModeManager->IsCoverPointTaken(CoverLocation);
}
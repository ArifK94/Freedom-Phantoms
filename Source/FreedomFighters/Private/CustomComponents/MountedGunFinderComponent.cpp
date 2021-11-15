// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/MountedGunFinderComponent.h"
#include "Weapons/MountedGun.h"

#include "Components/SphereComponent.h"


UMountedGunFinderComponent::UMountedGunFinderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SearchRadius = 500.0f;
	SearchLimit = 3;
}


void UMountedGunFinderComponent::BeginPlay()
{
	Super::BeginPlay();

	if (SearchSphere == nullptr)
	{
		SearchSphere = NewObject<USphereComponent>(GetOwner());
		if (SearchSphere)
		{
			SearchSphere->RegisterComponent();
			SearchSphere->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			SearchSphere->SetSphereRadius(SearchRadius);
			SearchSphere->SetCollisionProfileName(TEXT("OverlapAll"));
		}
	}
}

AMountedGun* UMountedGunFinderComponent::FindMG()
{
	AMountedGun* SelectedMG = nullptr;
	float TargetSightDistance = SearchRadius;

	TArray<AActor*> ActorsInSight;
	SearchSphere->GetOverlappingActors(ActorsInSight, AMountedGun::StaticClass());

	// The current limit of targets to process
	int CurrentSearchCount = 0;

	for (int index = 0; index < ActorsInSight.Num(); index++)
	{
		if (CurrentSearchCount > SearchLimit) {
			break;
		}

		AMountedGun* PotentialMG = Cast<AMountedGun>(ActorsInSight[index]);

		if (PotentialMG)
		{
			//check if mounted gun is present in the stronghold and has no owner as well as no potential owner in case another AI wishes to use it
			bool HasNoOwner = PotentialMG->GetOwner() == nullptr && PotentialMG->GetPotentialOwner() == nullptr;

			if (HasNoOwner && PotentialMG->GetCanTraceInteraction())
			{
				float DistanceDiff = FVector::Dist(GetOwner()->GetActorLocation(), PotentialMG->GetActorLocation());

				if (DistanceDiff < TargetSightDistance)
				{
					TargetSightDistance = DistanceDiff;
					SelectedMG = PotentialMG;
				}

				CurrentSearchCount++;
			}
		}
	}

	return SelectedMG;
}


// create a collision sphere
//FCollisionShape MyColSphere = FCollisionShape::MakeSphere(SearchRadius);

////create tarray for hit results
//TArray<FHitResult> OutHits;

////check if something got hit in the sweep
//bool isHit = GetWorld()->SweepMultiByChannel(OutHits, GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation(), FQuat::Identity, ECC_Visibility, MyColSphere);

//if (isHit)
//{
//	//loop through TArray
//	for (auto& Hit : OutHits)
//	{
//		AActor* HitActor = Hit.GetActor();

//		if (HitActor)
//		{
//			AMountedGun* PotentialMG = Cast<AMountedGun>(HitActor);

//			if (PotentialMG)
//			{
//				//check if mounted gun is present in the stronghold and has no owner as well as no potential owner in case another AI wishes to use it
//				bool HasNoOwner = PotentialMG->GetOwner() == nullptr && PotentialMG->GetPotentialOwner() == nullptr;
//				bool IsSamePotentialOwner = PotentialMG->GetPotentialOwner() == GetOwner();

//				if (HasNoOwner || IsSamePotentialOwner && PotentialMG->GetCanTraceInteraction())
//				{
//					FVector MGLocation = PotentialMG->GetActorLocation();

//					float DistanceDiff = FVector::Dist(GetOwner()->GetActorLocation(), MGLocation);

//					if (DistanceDiff < TargetSightDistance)
//					{
//						TargetSightDistance = DistanceDiff;
//						SelectedMG = PotentialMG;
//					}
//				}
//			}
//		}

//	}
//}

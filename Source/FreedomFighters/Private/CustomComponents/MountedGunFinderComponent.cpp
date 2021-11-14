// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/MountedGunFinderComponent.h"
#include "Weapons/MountedGun.h"

#include "Components/SphereComponent.h"


UMountedGunFinderComponent::UMountedGunFinderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SearchRadius = 500.0f;
}


void UMountedGunFinderComponent::BeginPlay()
{
	Super::BeginPlay();
}

AMountedGun* UMountedGunFinderComponent::FindMG()
{
	AMountedGun* SelectedMG = nullptr;
	float TargetSightDistance = SearchRadius;

	// create a collision sphere
	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(SearchRadius);

	//create tarray for hit results
	TArray<FHitResult> OutHits;

	//check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation(), FQuat::Identity, ECC_Visibility, MyColSphere);

	if (isHit)
	{
		//loop through TArray
		for (auto& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();

			if (HitActor)
			{
				AMountedGun* PotentialMG = Cast<AMountedGun>(HitActor);

				if (PotentialMG)
				{
					//check if mounted gun is present in the strongholdand has no owner as well as no potential owner in case another AI wishes to use it
					bool HasNoOwner = PotentialMG->GetOwner() == nullptr && PotentialMG->GetPotentialOwner() == nullptr;
					bool IsSamePotentialOwner = PotentialMG->GetPotentialOwner() != nullptr && PotentialMG->GetPotentialOwner() == GetOwner();

					if (HasNoOwner || IsSamePotentialOwner && PotentialMG->GetCanTraceInteraction())
					{
						FVector MGLocation = PotentialMG->GetActorLocation();

						float DistanceDiff = FVector::Dist(GetOwner()->GetActorLocation(), MGLocation);

						if (DistanceDiff < TargetSightDistance)
						{
							TargetSightDistance = DistanceDiff;
							SelectedMG = PotentialMG;
						}
					}
				}
			}

		}
	}








	return SelectedMG;
}


// ALTERNATIVE
// create a collision sphere
//FCollisionShape MyColSphere = FCollisionShape::MakeSphere(MountedGunSightRadius);
//
// create tarray for hit results
//TArray<FHitResult> OutHits;
//
// check if something got hit in the sweep
//bool isHit = GetWorld()->SweepMultiByChannel(OutHits, OwningCombatCharacter->GetActorLocation(), OwningCombatCharacter->GetActorLocation(), FQuat::Identity, ECC_Visibility, MyColSphere);
//
//if (isHit)
//{
//	 loop through TArray
//	for (auto& Hit : OutHits)
//	{
//		AActor* HitActor = Hit.GetActor();
//
//		if (HitActor)
//		{
//			AMountedGun* PotentialMG = Cast<AMountedGun>(HitActor);
//
//			if (PotentialMG)
//			{
//				 check if mounted gun is present in the stronghold and has no owner as well as no potential owner in case another AI wishes to use it
//				bool HasNoOwner = PotentialMG->GetOwner() == nullptr && PotentialMG->GetPotentialOwner() == nullptr;
//				bool IsSamePotentialOwner = PotentialMG->GetPotentialOwner() != nullptr && PotentialMG->GetPotentialOwner() == OwningCombatCharacter;
//
//				if (HasNoOwner || IsSamePotentialOwner && PotentialMG->GetCanTraceInteraction())
//				{
//					FVector MGLocation = PotentialMG->GetActorLocation();
//
//					float DistanceDiff = FVector::Dist(OwningCombatCharacter->GetActorLocation(), MGLocation);
//
//					if (DistanceDiff < TargetSightDistance)
//					{
//						TargetSightDistance = DistanceDiff;
//						SelectedMG = PotentialMG;
//					}
//				}
//			}
//		}
//
//	}
//}
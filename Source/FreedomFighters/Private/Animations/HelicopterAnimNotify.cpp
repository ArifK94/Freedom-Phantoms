// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/HelicopterAnimNotify.h"
#include "Characters/BaseCharacter.h"
#include "Vehicles/Aircraft.h"

#include "Components/CapsuleComponent.h"

void UHelicopterAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner());

		if (Character)
		{
			FAircraftSeating MySeat = Character->GetAircraftSeat();
			AAircraft* OwningAircraft = MySeat.OwningAircraft;

			if (DetachFromHelicopter)
			{
				Character->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				Character->SetAircraftSeat(FAircraftSeating());
				Character->SetIsRepellingDown(false);
			}

			if (IsRopeFreeToUse)
			{
				if (OwningAircraft)
				{
					if (MySeat.isRopeLeftSide)
					{
						OwningAircraft->IsLeftRappelOccupied(false);
					}
					else
					{
						OwningAircraft->IsRightRappelOccupied(false);
					}
				}
			}
		}
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/HelicopterAnimNotify.h"

#include "Characters/BaseCharacter.h"

#include "Vehicles/Helicopter.h"


void UHelicopterAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner());

		if (Character)
		{
			FHelicopterSeating MySeat = Character->CurrentSeating();
			AHelicopter* OwningHelicopter = MySeat.OwningHelicopter;

			if (DetachFromHelicopter)
			{
				Character->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				Character->SetIsInHelicopter(false);
				Character->SetIsRepellingDown(false);
			}

			if (IsRopeFreeToUse)
			{
				if (OwningHelicopter)
				{
					if (MySeat.isRopeLeftSide)
					{
						OwningHelicopter->IsLeftRappelOccupied(false);
					}
					else
					{
						OwningHelicopter->IsRightRappelOccupied(false);
					}
				}
			}
		}
	}
}

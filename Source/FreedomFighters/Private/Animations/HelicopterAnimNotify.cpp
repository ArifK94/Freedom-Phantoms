// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/HelicopterAnimNotify.h"
#include "Characters/BaseCharacter.h"
#include "Accessories/Rope.h"
#include "CustomComponents/VehiclePathFollowerComponent.h"

#include "Components/CapsuleComponent.h"

void UHelicopterAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner());

		if (Character)
		{
			if (DetachFromHelicopter)
			{
				Character->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				Character->SetVehicleSeat(FVehicletSeating());
			}

			if (IsRopeFreeToUse)
			{
				UVehiclePathFollowerComponent::SetRopeFree(Character->GetVehicletSeat());
			}
		}
	}
}

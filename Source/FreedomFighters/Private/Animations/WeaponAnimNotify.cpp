// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/WeaponAnimNotify.h"

#include "Characters/CombatCharacter.h"
#include "Components/SkeletalMeshComponent.h"

#include "Weapons/Shotgun.h"


void UWeaponAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		ACombatCharacter* Character = Cast<ACombatCharacter>(MeshComp->GetOwner());

		if (Character)
		{
			switch (animType)
			{
			case AnimType::Grab:
				Character->GrabWeapon();
				break;
			case AnimType::EndEquip:
				Character->EndEquipWeapon();
				break;
			case AnimType::UnEquip:
				Character->unEquipWeapon();
				break;
			case AnimType::SwapWeapon:
				Character->swapWeapon();
				break;
			default:
				break;
			}

		}
	}
}

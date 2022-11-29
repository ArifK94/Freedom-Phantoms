// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/WeaponAnimNotify.h"

#include "Characters/CombatCharacter.h"
#include "Components/SkeletalMeshComponent.h"

#include "Weapons/Weapon.h"


void UWeaponAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		ACombatCharacter* Character = Cast<ACombatCharacter>(MeshComp->GetOwner());

		if (Character)
		{
			switch (animType)
			{
			case AnimType::GrabWeapon:
				Character->GrabWeapon();
				break;
			case AnimType::EndEquip:
				Character->EndEquipWeapon();
				break;
			case AnimType::UnEquip:
				Character->HolsterWeapon();
				break;
			case AnimType::SwapWeapon:
				Character->swapWeapon();
				break;
			case AnimType::ReloadEnd:
				Character->EndReload();
			default:
				break;
			}

			AWeapon* Weapon = Character->GetCurrentWeapon();

			if (Weapon)
			{
				switch (animType)
				{
				case AnimType::ReloadClipIn:
					Weapon->ClipIn();
					break;
				case AnimType::ReloadClipOut:
					Weapon->ClipOut();
					break;
				case AnimType::GrabClip:
					Character->GetCurrentWeapon()->SetClipSocket(Character->GetMesh());
					break;
				case AnimType::FireWeapon:
					Weapon->Fire();
					break;
				case AnimType::StopFire:
					Weapon->StopFire();
					break;
				default:
					break;
				}

			}


		}
	}
}

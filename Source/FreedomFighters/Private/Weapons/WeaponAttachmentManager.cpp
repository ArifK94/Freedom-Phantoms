// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponAttachmentManager.h"
#include "Weapons/Weapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "UObject/UObjectGlobals.h"

UWeaponAttachmentManager::UWeaponAttachmentManager()
{
}

void UWeaponAttachmentManager::SpawnAttachments(USkeletalMeshComponent* mesh, AActor* owner, UWorld* ActorWorld)
{
	World = ActorWorld;
	SpawnUnderBarrel(mesh, owner);
}

void UWeaponAttachmentManager::SpawnUnderBarrel(USkeletalMeshComponent* mesh, AActor* owner)
{
	if (World)
	{
		int RandIndex = rand() % UnderBarrelWeaponClasses.Num();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		// Spawn the weapon actor
		UnderBarrelWeaponObj = World->SpawnActor<AWeapon>(UnderBarrelWeaponClasses[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (UnderBarrelWeaponObj)
		{
			UnderBarrelWeaponObj->SetOwner(owner);
			UnderBarrelWeaponObj->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, UnderBarrelWeaponObj->getWeaponHandSocket());
		}
	}
}


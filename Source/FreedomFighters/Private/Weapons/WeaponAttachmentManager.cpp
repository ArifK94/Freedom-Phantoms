// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponAttachmentManager.h"
#include "Weapons/Weapon.h"
#include "Weapons/WeaponOptic.h"
#include "Weapons/WeaponLaser.h"
#include "Weapons/WeaponTorchlight.h"


#include "Components/SkeletalMeshComponent.h"
#include "UObject/UObjectGlobals.h"

UWeaponAttachmentManager::UWeaponAttachmentManager()
{
}

void UWeaponAttachmentManager::SpawnAttachments(USkeletalMeshComponent* mesh, AWeapon* owner, UWorld* ActorWorld)
{
	World = ActorWorld;
	SpawnUnderBarrel(mesh, owner);
	SpawnOptics(mesh, owner);
	SpawnLaser(mesh, owner);
	SpawnTorch(mesh, owner);

}

void UWeaponAttachmentManager::SpawnUnderBarrel(USkeletalMeshComponent* mesh, AWeapon* owner)
{
	if (UnderBarrelWeaponClasses.Num() > 0)
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

}


void UWeaponAttachmentManager::SpawnOptics(USkeletalMeshComponent* mesh, AWeapon* owner)
{
	if (OpticClasses.Num() > 0)
	{
		if (World)
		{
			int RandIndex = rand() % OpticClasses.Num();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = owner;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


			// Spawn the weapon actor
			OpticObj = World->SpawnActor<AWeaponOptic>(OpticClasses[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (OpticObj)
			{
				OpticObj->SetOwner(owner);
				OpticObj->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, owner->getOpticsSocket());
			}
		}
	}

}

void UWeaponAttachmentManager::SpawnLaser(USkeletalMeshComponent* mesh, AWeapon* owner)
{
	if (LaserClasses.Num() > 0)
	{
		if (World)
		{
			int RandIndex = rand() % LaserClasses.Num();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = owner;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


			// Spawn the weapon actor
			LaserObj = World->SpawnActor<AWeaponLaser>(LaserClasses[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (LaserObj)
			{
				LaserObj->SetOwner(owner);
				LaserObj->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, owner->getLaserSocket());
			}
		}
	}
}



void UWeaponAttachmentManager::SpawnTorch(USkeletalMeshComponent* mesh, AWeapon* owner)
{
	if (TorchlightClasses.Num() > 0)
	{
		if (World)
		{
			int RandIndex = rand() % TorchlightClasses.Num();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = owner;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


			// Spawn the weapon actor
			TorchlightObj = World->SpawnActor<AWeaponTorchlight>(TorchlightClasses[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (TorchlightObj)
			{
				TorchlightObj->SetOwner(owner);
				TorchlightObj->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, owner->getTorchlightSocket());
			}
		}
	}

}

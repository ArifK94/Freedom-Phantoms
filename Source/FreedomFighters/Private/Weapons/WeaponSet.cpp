// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponSet.h"

#include "Accessories/Headgear.h"
#include "Accessories/Loadout.h"

#include "Weapons/Weapon.h"
#include "Weapons/AssaultRifle.h"
#include "Weapons/LMG.h"
#include "Weapons/Shotgun.h"
#include "Weapons/SMG.h"

#include "FreedomFighters/FreedomFighters.h"

#include "Particles/ParticleSystem.h"

#include "Components/CapsuleComponent.h"

#include "Components/SkeletalMeshComponent.h"

#include <array>


UWeaponSet::UWeaponSet()
{
}

void UWeaponSet::Init(UWorld* World)
{
}

AWeapon* UWeaponSet::SpawnAssaultRifle(UWorld* world, USkeletalMeshComponent* mesh, AActor* owner)
{
	if (world)
	{
		int RandIndex = rand() % AssaultRifles.Num();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Spawn the weapon actor
		weaponPrimaryObj = world->SpawnActor<AWeapon>(AssaultRifles[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);


		if (weaponPrimaryObj)
		{
			weaponPrimaryObj->SetOwner(owner);
			weaponPrimaryObj->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, weaponPrimaryObj->getHolsterSocket());

		}
	}
	return weaponPrimaryObj;
}

AWeapon* UWeaponSet::SpawnSMG(UWorld* world, USkeletalMeshComponent* mesh, AActor* owner)
{
	if (world)
	{
		int RandIndex = rand() % SMGs.Num();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Spawn the weapon actor
		weaponPrimaryObj = world->SpawnActor<AWeapon>(SMGs[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (weaponPrimaryObj)
		{
			weaponPrimaryObj->SetOwner(owner);
			weaponPrimaryObj->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, weaponPrimaryObj->getHolsterSocket());
		}
	}
	return weaponPrimaryObj;

}


AWeapon* UWeaponSet::SpawnShotgun(UWorld* world, USkeletalMeshComponent* mesh, AActor* owner)
{
	if (world)
	{
		int RandIndex = rand() % Shotguns.Num();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Spawn the weapon actor
		weaponPrimaryObj = world->SpawnActor<AWeapon>(Shotguns[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (weaponPrimaryObj)
		{
			weaponPrimaryObj->SetOwner(owner);
			weaponPrimaryObj->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, weaponPrimaryObj->getHolsterSocket());
		}
	}
	return weaponPrimaryObj;

}

AWeapon* UWeaponSet::SpawnLMG(UWorld* world, USkeletalMeshComponent* mesh, AActor* owner)
{
	if (world)
	{
		int RandIndex = rand() % LMGs.Num();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Spawn the weapon actor
		weaponPrimaryObj = world->SpawnActor<AWeapon>(LMGs[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (weaponPrimaryObj)
		{
			weaponPrimaryObj->SetOwner(owner);
			weaponPrimaryObj->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, weaponPrimaryObj->getHolsterSocket());
		}
	}
	return weaponPrimaryObj;

}

AWeapon* UWeaponSet::SpawnSecondaryWeapon(UWorld* world, USkeletalMeshComponent* mesh, AActor* owner)
{
	if (world)
	{
		int RandIndex = rand() % SecondaryWeapons.Num();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Spawn the weapon actor
		weaponSecondaryObj = world->SpawnActor<AWeapon>(SecondaryWeapons[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (weaponSecondaryObj)
		{
			weaponSecondaryObj->SetOwner(owner);
			weaponSecondaryObj->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, weaponSecondaryObj->getHolsterSocket());
		}
	}
	return weaponSecondaryObj;
}
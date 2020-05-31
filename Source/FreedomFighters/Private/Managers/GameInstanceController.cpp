// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/GameInstanceController.h"

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


AWeapon* UGameInstanceController::SpawnAssaultRifle(USkeletalMeshComponent* mesh, AActor* owner)
{
	//return SpawnWeapon(mesh, owner, AssaultRifles);
	UWorld* world = GetWorld();

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

AWeapon* UGameInstanceController::SpawnSMG(USkeletalMeshComponent* mesh, AActor* owner)
{
	UWorld* world = GetWorld();

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


AWeapon* UGameInstanceController::SpawnShotgun(USkeletalMeshComponent* mesh, AActor* owner)
{
	UWorld* world = GetWorld();

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

AWeapon* UGameInstanceController::SpawnLMG(USkeletalMeshComponent* mesh, AActor* owner)
{
	UWorld* world = GetWorld();

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

AWeapon* UGameInstanceController::SpawnSecondaryWeapon(USkeletalMeshComponent* mesh, AActor* owner)
{
	UWorld* world = GetWorld();

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
			weaponSecondaryObj->SetActorRelativeLocation(holsterSideArmLocation);
			weaponSecondaryObj->SetActorRelativeRotation(holsterSideArmRotation);
		}
	}
	return weaponSecondaryObj;
}


AWeapon* UGameInstanceController::SpawnPistol(USkeletalMeshComponent* mesh, AActor* owner)
{
	return nullptr;
}

UParticleSystem* UGameInstanceController::CheckSurface(EPhysicalSurface SurfaceType)
{
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		return FleshImpactEffect;
		break;
	default:
		return DefaultImpactEffect;
		break;
	}
}

void UGameInstanceController::setHolsterPrimaryLocation(FVector location)
{
	holsterPrimaryLocation = location;
}

void UGameInstanceController::setHolsterPrimaryRotation(FRotator rotation)
{
	holsterPrimaryRotation = rotation;
}


void UGameInstanceController::setHolsterSideArmLocation(FVector location)
{
	holsterSideArmLocation = location;
}

void UGameInstanceController::setHolsterSideArmRotation(FRotator rotation)
{
	holsterSideArmRotation = rotation;
}
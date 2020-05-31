#include "Managers/GameManager.h"

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


void AGameManager::SpawnHelmet(USkeletalMeshComponent* mesh, AActor* owner)
{
	UWorld* world = GetWorld();

	if (world)
	{
		int RandIndex = rand() % headgears.Num();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		// Spawn a random helmet actor
		headgearObj = world->SpawnActor<AHeadgear>(headgears[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (headgearObj)
		{
			headgearObj->SetOwner(owner);
			headgearObj->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Headgear");
		}
	}
}

ALoadout* AGameManager::SpawnLoadout(USkeletalMeshComponent* mesh, AActor* owner)
{
	UWorld* world = GetWorld();

	if (world)
	{
		int RandIndex = rand() % loadouts.Num();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		// Spawn a random helmet actor
		loadoutObj = world->SpawnActor<ALoadout>(loadouts[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (loadoutObj)
		{
			loadoutObj->SetOwner(owner);
			loadoutObj->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "loadout_socket");
			loadoutObj->getLoadoutMesh()->SetMasterPoseComponent(mesh);
		}
	}
	return loadoutObj;
}

AWeapon* AGameManager::SpawnWeapon(USkeletalMeshComponent* mesh, AActor* owner, TArray<TSubclassOf<class AWeapon>> weapons)
{
	UWorld* world = GetWorld();

	if (world)
	{
		int RandIndex = rand() % weapons.Num();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Spawn the weapon actor
		weaponPrimaryObj = world->SpawnActor<AWeapon>(weapons[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (weaponPrimaryObj)
		{
			weaponPrimaryObj->SetOwner(owner);
			weaponPrimaryObj->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, weaponPrimaryObj->getHolsterSocket());
		}
	}
	return weaponSecondaryObj;
}



AWeapon* AGameManager::SpawnAssaultRifle(USkeletalMeshComponent* mesh, AActor* owner)
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

AWeapon* AGameManager::SpawnSMG(USkeletalMeshComponent* mesh, AActor* owner)
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


AWeapon* AGameManager::SpawnShotgun(USkeletalMeshComponent* mesh, AActor* owner)
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

AWeapon* AGameManager::SpawnLMG(USkeletalMeshComponent* mesh, AActor* owner)
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

AWeapon* AGameManager::SpawnSecondaryWeapon(USkeletalMeshComponent* mesh, AActor* owner)
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


AWeapon* AGameManager::SpawnPistol(USkeletalMeshComponent* mesh, AActor* owner)
{
	return nullptr;
}

UParticleSystem* AGameManager::CheckSurface(EPhysicalSurface SurfaceType)
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

void AGameManager::setHolsterPrimaryLocation(FVector location)
{
	holsterPrimaryLocation = location;
}

void AGameManager::setHolsterPrimaryRotation(FRotator rotation)
{
	holsterPrimaryRotation = rotation;
}


void AGameManager::setHolsterSideArmLocation(FVector location)
{
	holsterSideArmLocation = location;
}

void AGameManager::setHolsterSideArmRotation(FRotator rotation)
{
	holsterSideArmRotation = rotation;
}
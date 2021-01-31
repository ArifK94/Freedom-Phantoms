

#include "Managers/FactionManager.h"
#include "Weapons/WeaponSet.h"

#include "Accessories/Headgear.h"
#include "Accessories/Loadout.h"

#include "Components/SkeletalMeshComponent.h"

#include "UObject/UObjectGlobals.h"
#include <Runtime\Engine\Classes\Engine\World.h>
#include <array>

#include "EngineUtils.h"

UFactionManager::UFactionManager()
{

}

void UFactionManager::Init(UWorld* World)
{
	CurrentWorld = World;

	if (WeaponSetClass)
		WeaponSetObj = NewObject<UWeaponSet>((UObject*)GetTransientPackage(), WeaponSetClass);

	setRanomVoiceSet();
}

void UFactionManager::setRanomVoiceSet()
{
	if (VoiceClips.Num() > 0)
	{
		int RandIndex = rand() % VoiceClips.Num();

		SelectedVoiceClipSet = VoiceClips[RandIndex];
	}
}

AHeadgear* UFactionManager::SpawnHelmet(USkeletalMeshComponent* Mesh, AActor* Owner)
{
	if (Headgears.Num() > 0)
	{
		if (CurrentWorld)
		{
			int RandIndex = rand() % Headgears.Num();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Owner;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


			// Spawn a random helmet actor
			headgearObj = CurrentWorld->SpawnActor<AHeadgear>(Headgears[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (headgearObj)
			{
				headgearObj->SetOwner(Owner);
				headgearObj->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "headgear_socket");
				return headgearObj;

			}
		}
	}

	return nullptr;

}


ALoadout* UFactionManager::SpawnLoadout(USkeletalMeshComponent* Mesh, AActor* Owner)
{
	if (CurrentWorld)
	{
		int RandIndex = rand() % Loadouts.Num();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		// Spawn a random helmet actor
		loadoutObj = CurrentWorld->SpawnActor<ALoadout>(Loadouts[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (loadoutObj)
		{
			loadoutObj->SetOwner(Owner);
			loadoutObj->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			loadoutObj->getLoadoutMesh()->SetMasterPoseComponent(Mesh);
			loadoutObj->Init(WeaponSetObj);

			return loadoutObj;
		}
	}
	return nullptr;
}
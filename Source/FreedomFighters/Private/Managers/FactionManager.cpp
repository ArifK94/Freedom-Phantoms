

#include "Managers/FactionManager.h"
#include "Weapons/WeaponSet.h"

#include "Accessories/Headgear.h"
#include "Accessories/Loadout.h"

#include "UObject/UObjectGlobals.h"
#include <Runtime\Engine\Classes\Engine\World.h>

void UFactionManager::Init(UWorld* World)
{
	WorldOwner = World;
	WeaponSetObj = NewObject<UWeaponSet>((UObject*)GetTransientPackage(), WeaponSetClass);

}

void UFactionManager::SetWeaponTypeClass(USkeletalMeshComponent* mesh, AActor* owner)
{

}

void UFactionManager::SpawnHelmet()
{
	//if (WorldOwner)
	//{
	//	int RandIndex = rand() % Headgears.Num();

	//	FActorSpawnParameters SpawnParams;
	//	SpawnParams.Owner = WorldOwner;
	//	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


	//	// Spawn a random helmet actor
	//	headgearObj = world->SpawnActor<AHeadgear>(Headgears[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	//	if (headgearObj)
	//	{
	//		headgearObj->SetOwner(WorldOwner);
	//		headgearObj->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "headgear_socket");
	//	}
	//}
}


void UFactionManager::SpawnLoadout()
{
	//if (WorldOwner)
	//{
	//	int RandIndex = rand() % Loadouts.Num();

	//	FActorSpawnParameters SpawnParams;
	//	SpawnParams.Owner = WorldOwner;
	//	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


	//	// Spawn a random helmet actor
	//	loadoutObj = world->SpawnActor<ALoadout>(Loadouts[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	//	if (loadoutObj)
	//	{
	//		loadoutObj->SetOwner(WorldOwner);
	//		loadoutObj->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	//		loadoutObj->getLoadoutMesh()->SetMasterPoseComponent(GetMesh());
	//	}
	//}
}



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
}
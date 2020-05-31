// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessories/Loadout.h"

#include "Managers/GameInstanceController.h"
#include "Weapons/Weapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Engine.h"

// Sets default values
ALoadout::ALoadout()
{
	CreateSkeletalMeshParent();

	FRotator MakeControlRot = UKismetMathLibrary::MakeRotator(0.0f, 0.0f, 270.0f);
	SkelMesh->SetRelativeRotation(MakeControlRot);

	ParentSocket = "loadout_socket";
}


void ALoadout::BeginPlay()
{
	Super::BeginPlay();

	UWorld* world = GetWorld();
	UGameInstance* instance = UGameplayStatics::GetGameInstance(world);
	gameInstanceController = Cast<UGameInstanceController>(instance);

}

AWeapon* ALoadout::SpawnPrimaryWeapon(USkeletalMeshComponent* mesh, AActor* owner)
{
	switch (loadoutType)
	{
	case LoadoutType::Assault:
		return gameInstanceController->SpawnAssaultRifle(mesh, owner);
		break;
	case LoadoutType::SMG:
		return gameInstanceController->SpawnSMG(mesh, owner);
		break;
	case LoadoutType::Shotgun:
	//	return gameInstanceController->SpawnShotgun(mesh, owner);
		return gameInstanceController->SpawnAssaultRifle(mesh, owner);

		break;
	case LoadoutType::LMG:
		return	gameInstanceController->SpawnLMG(mesh, owner);
		break;
	default:
		return gameInstanceController->SpawnAssaultRifle(mesh, owner);
		break;
	}
	return	gameInstanceController->SpawnAssaultRifle(mesh, owner);
}

FVector ALoadout::getPrimaryWeaponHolsterLocation()
{
	return PrimaryWeaponHolsterLocation;
}


FRotator ALoadout::getPrimaryWeaponHolsterRotation()
{
	return PrimaryWeaponHolsterRotation;
}

FVector ALoadout::getSideArmHolsterLocation()
{
	return SideArmHolsterLocation;
}

FRotator ALoadout::getSideArmHolsterRotation()
{
	return SideArmHolsterRotation;
}




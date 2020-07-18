// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessories/Loadout.h"

#include "Managers/GameInstanceController.h"
#include "Weapons/Weapon.h"
#include "Weapons/WeaponSet.h"
#include "Weapons/WeaponAttachmentManager.h"

#include "UObject/UObjectGlobals.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Engine.h"



// Sets default values
ALoadout::ALoadout()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	Mesh->CanCharacterStepUpOn = ECB_No;


	FRotator MakeControlRot = UKismetMathLibrary::MakeRotator(0.0f, 0.0f, 270.0f);
	Mesh->SetRelativeRotation(MakeControlRot);
}


void ALoadout::BeginPlay()
{
	Super::BeginPlay();

	UWorld* world = GetWorld();
	UGameInstance* instance = UGameplayStatics::GetGameInstance(world);
	gameInstanceController = Cast<UGameInstanceController>(instance);


	WeaponSetObj = NewObject<UWeaponSet>((UObject*)GetTransientPackage(), WeaponSetClass);
}

AWeapon* ALoadout::SpawnPrimaryWeapon(USkeletalMeshComponent* mesh, AActor* owner)
{
	UWorld* World = GetWorld();

	switch (loadoutType)
	{
	case LoadoutType::Assault:
		return WeaponSetObj->SpawnAssaultRifle(World, mesh, owner);
		break;
	case LoadoutType::SMG:
		return WeaponSetObj->SpawnSMG(World, mesh, owner);
		break;
	case LoadoutType::Shotgun:
		return WeaponSetObj->SpawnShotgun(World, mesh, owner);
		break;
	case LoadoutType::LMG:
		return WeaponSetObj->SpawnLMG(World, mesh, owner);
		break;
	default:
		return WeaponSetObj->SpawnAssaultRifle(World, mesh, owner);
		break;
	}
}
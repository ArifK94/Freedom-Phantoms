// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessories/Loadout.h"

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

	//UWorld* world = GetWorld();
	//UGameInstance* instance = UGameplayStatics::GetGameInstance(world);
	//gameInstanceController = Cast<UGameInstanceController>(instance);
}

void ALoadout::Init(UWeaponSet* WeaponSetObj)
{
	CurrentWeaponSetObj = WeaponSetObj;
}

AWeapon* ALoadout::SpawnPrimaryWeapon(USkeletalMeshComponent* mesh, AActor* owner)
{
	UWorld* World = GetWorld();

	switch (loadoutType)
	{
	case LoadoutType::Assault:
		return CurrentWeaponSetObj->SpawnAssaultRifle(World, mesh, owner);
		break;
	case LoadoutType::SMG:
		return CurrentWeaponSetObj->SpawnSMG(World, mesh, owner);
		break;
	case LoadoutType::Shotgun:
		return CurrentWeaponSetObj->SpawnShotgun(World, mesh, owner);
		break;
	case LoadoutType::LMG:
		return CurrentWeaponSetObj->SpawnLMG(World, mesh, owner);
		break;
	default:
		return CurrentWeaponSetObj->SpawnAssaultRifle(World, mesh, owner);
		break;
	}
}
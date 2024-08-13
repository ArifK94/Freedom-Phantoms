// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessories/Loadout.h"
#include "StructCollection.h"
#include "Weapons/Weapon.h"
#include "Weapons/ThrowableWeapon.h"

#include "UObject/UObjectGlobals.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Engine.h"

ALoadout::ALoadout()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("Loadout"));
	Mesh->CanCharacterStepUpOn = ECB_No;
	RootComponent = Mesh;

	FRotator MakeControlRot = UKismetMathLibrary::MakeRotator(0.0f, 0.0f, 270.0f);
	Mesh->SetRelativeRotation(MakeControlRot);

	UseMasterPoseComponent = false;
}

void ALoadout::BeginPlay()
{
	Super::BeginPlay();

	SimlateBones();
}

void ALoadout::SimlateBones()
{
	for (int i = 0; i < PhysicsBones.Num(); i++)
	{
		FName Bone = PhysicsBones[i];

		Mesh->SetAllBodiesBelowSimulatePhysics(Bone, true, true);
	}
}

AWeapon* ALoadout::SpawnWeapon(FWeaponsSet* WeaponsDataSet, bool IsPrimary)
{
	if (!WeaponsDataSet) {
		return nullptr;
	}

	int RandomIndex = 0;
	AWeapon* Weapon = nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (IsPrimary)
	{
		switch (loadoutType)
		{
		case LoadoutType::SMG:
			RandomIndex = rand() % WeaponsDataSet->SMGs.Num();
			Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponsDataSet->SMGs[RandomIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			break;
		case LoadoutType::Shotgun:
			RandomIndex = rand() % WeaponsDataSet->Shotguns.Num();
			Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponsDataSet->Shotguns[RandomIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			break;
		case LoadoutType::LMG:
			RandomIndex = rand() % WeaponsDataSet->LMGs.Num();
			Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponsDataSet->LMGs[RandomIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			break;
		default: // Assault loadout by default
			RandomIndex = rand() % WeaponsDataSet->AssaultRifles.Num();
			Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponsDataSet->AssaultRifles[RandomIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			break;
		}
	}
	else
	{
		if (rand() % 1 == 0)
		{
			RandomIndex = rand() % WeaponsDataSet->Handguns.Num();
			Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponsDataSet->Handguns[RandomIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}
		else
		{
			RandomIndex = rand() % WeaponsDataSet->MachinePistols.Num();
			Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponsDataSet->MachinePistols[RandomIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}
	}



	if (Weapon)
	{
		Weapon->SetOwner(GetOwner());
		HolsterWeapon(Weapon);
	}

	return Weapon;
}

AWeapon* ALoadout::SpawnWeapon(TSubclassOf<AWeapon> WeaponClass, bool IsPrimary)
{
	if (!WeaponClass) {
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (!IsPrimary)
	{
		HolsterWeapon(Weapon);
	}

	return Weapon;
}

AThrowableWeapon* ALoadout::SpawnGrenade(FWeaponsSet* WeaponsDataSet)
{
	if (!WeaponsDataSet || !WeaponsDataSet->GrenadeClass) {
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	auto GrenadeWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(WeaponsDataSet->GrenadeClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (GrenadeWeapon) {
		GrenadeWeapon->ToggleVisibility(false);
		//GrenadeWeapon->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	return GrenadeWeapon;
}

void ALoadout::HolsterWeapon(AWeapon* Weapon)
{
	if (Weapon) {
		Weapon->HolsterWeapon(Mesh);
	}
}
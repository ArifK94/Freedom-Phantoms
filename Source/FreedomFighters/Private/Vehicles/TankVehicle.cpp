// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/TankVehicle.h"
#include "Weapons/MountedGun.h"

ATankVehicle::ATankVehicle()
{

}

void ATankVehicle::BeginPlay()
{
	Super::BeginPlay();

	SpawnWeapons();
}

void ATankVehicle::SpawnWeapons()
{
	if (VehicleWeapons.Num() <= 0) {
		return;
	}

	auto MyOwner = GetOwner() ? GetOwner() : this;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = MyOwner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int i = 0; i < VehicleWeapons.Num(); i++)
	{
		auto VehicleWeapon = VehicleWeapons[i];

		auto Weapon = GetWorld()->SpawnActor<AMountedGun>(VehicleWeapon.WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (!Weapon) {
			continue;
		}

		Weapon->SetOwner(MyOwner);
		Weapon->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleWeapon.WeaponSocketName);
		WeaponsCollection.Add(Weapon);
	}

	CurrentWeapon = WeaponsCollection[0];
}


void ATankVehicle::SetRotationInput(FRotator Rotation)
{
	RotationInput = Rotation;
}
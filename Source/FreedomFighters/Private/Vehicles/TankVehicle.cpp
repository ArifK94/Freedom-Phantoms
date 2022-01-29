// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/TankVehicle.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"
#include "CustomComponents/ShooterComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "CustomComponents/TargetFinderComponent.h"

void ATankVehicle::SetRotationInput(FRotator Rotation)
{
	RotationInput = Rotation;
}

ATankVehicle::ATankVehicle()
{
	TeamFactionComponent = CreateDefaultSubobject<UTeamFactionComponent>(TEXT("TeamFactionComponent"));

	TargetFinderComponent = CreateDefaultSubobject<UTargetFinderComponent>(TEXT("TargetFinderComponent"));
	TargetFinderComponent->SetFindTargetPerFrame(true);

	ShooterComponent = CreateDefaultSubobject<UShooterComponent>(TEXT("ShooterComponent"));

	CurrentWeaponIndex = 0;
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
	ShooterComponent->SetWeapon(CurrentWeapon);
}

void ATankVehicle::ChangeWeapon()
{
	if (VehicleWeapons.Num() <= 0) {
		return;
	}

	// increment the index if current index is less than the array of weapons
	// otherwise go back to the first index
	if (CurrentWeaponIndex < VehicleWeapons.Num() - 1)
	{
		CurrentWeaponIndex++;
	}
	else
	{
		CurrentWeaponIndex = 0;
	}

	if (CurrentWeapon) {
		CurrentWeapon->StopFire(); // stop firing current weapon before switching to another
	}

	ShooterComponent->SetWeapon(CurrentWeapon);
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/TankVehicle.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"
#include "CustomComponents/ShooterComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "CustomComponents/TargetFinderComponent.h"

void ATankVehicle::SetRotationInput(FRotator InRotation)
{
	//RotationInput.Pitch = FMath::ClampAngle(RotationInput.Pitch + InRotation.Pitch, PitchMin, PitchMax);
	//RotationInput.Pitch = FRotator::ClampAxis(RotationInput.Pitch);

	//RotationInput.Yaw = FMath::ClampAngle(RotationInput.Yaw + InRotation.Yaw, YawMin, YawMax);
	//RotationInput.Yaw = FRotator::ClampAxis(RotationInput.Yaw);

	RotationInput = InRotation;
}

ATankVehicle::ATankVehicle()
{
	TeamFactionComponent = CreateDefaultSubobject<UTeamFactionComponent>(TEXT("TeamFactionComponent"));

	TargetFinderComponent = CreateDefaultSubobject<UTargetFinderComponent>(TEXT("TargetFinderComponent"));
	TargetFinderComponent->SetFindTargetPerFrame(true);
	TargetFinderComponent->AddClassFilter(ATankVehicle::StaticClass());

	ShooterComponent = CreateDefaultSubobject<UShooterComponent>(TEXT("ShooterComponent"));

	CurrentWeaponIndex = 0;
}

void ATankVehicle::BeginPlay()
{
	Super::BeginPlay();

	SpawnVehicleWeapon(VehicleWeaponMain);

	for (int i = 0; i < VehicleWeaponTurrets.Num(); i++)
	{
		SpawnVehicleWeapon(VehicleWeaponTurrets[i]);
	}

	if (WeaponsCollection.Num() > 0) {
		CurrentWeapon = WeaponsCollection[0];
		ShooterComponent->SetWeapon(CurrentWeapon);
	}

}

void ATankVehicle::SpawnVehicleWeapon(FVehicleWeapon VehicleWeapon)
{
	if (!VehicleWeapon.WeaponClass) {
		return;
	}

	auto MyOwner = GetOwner() ? GetOwner() : this;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = MyOwner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	auto Weapon = GetWorld()->SpawnActor<AMountedGun>(VehicleWeapon.WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (!Weapon) {
		return;
	}

	Weapon->SetOwner(MyOwner);
	Weapon->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleWeapon.WeaponSocketName);
	WeaponsCollection.Add(Weapon);
}

void ATankVehicle::ChangeWeapon()
{
	if (WeaponsCollection.Num() <= 0) {
		return;
	}

	// increment the index if current index is less than the array of weapons
	// otherwise go back to the first index
	if (CurrentWeaponIndex < WeaponsCollection.Num() - 1)
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
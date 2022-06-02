// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ThrowableWeapon.h"

AThrowableWeapon::AThrowableWeapon()
{
	weaponType = WeaponType::Throwable;

	AmmoPerClip = 1;
	MaxAmmoCapacity = 5;

	hasRecoil = false;
	CanAutoReload = true;
}

FString AThrowableWeapon::OnInteractionFound_Implementation(APawn* InPawn, AController* InController)
{
	return "";
}

AActor* AThrowableWeapon::OnPickup_Implementation(APawn* InPawn, AController* InController)
{
	return nullptr;
}

void AThrowableWeapon::SetIsAiming(bool isAiming)
{
	Super::SetIsAiming(isAiming);

	if (isAiming)
	{

	}
}

/**
* Let the animation / montage take care of throwing the projectile since the projectile should when character is throwing.
*/
void AThrowableWeapon::Fire()
{
	if (isReloading) {
		return;
	}

	if (CurrentAmmo <= 0) {
		isFiring = false;

		OnEmptyAmmoClip.Broadcast(this);

		if (!HasNoReload) {
			return;
		}
	}

	HasFiredFirstShot = true;
	CurrentAmmo -= 1;

	CreateBullet();

	if (CurrentAmmo <= 0) {
		isFiring = false;

		OnEmptyAmmoClip.Broadcast(this);

		if (!HasNoReload) {
			return;
		}
	}
}

void AThrowableWeapon::StartFire()
{
	if (isFiring) {
		return;
	}

	if (isReloading) {
		return;
	}

	isFiring = false;
}
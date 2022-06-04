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

	if (isAiming) {
		if (CurrentAmmo > 0) {
			MeshComp->SetHiddenInGame(false, true);
		}
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

	EmptyClipEvent();

	if (CurrentAmmo <= 0) {
		return;
	}

	HasFiredFirstShot = true;
	CurrentAmmo -= 1;

	CreateBullet();

	EmptyClipEvent();

	MeshComp->SetHiddenInGame(true, true);
}

void AThrowableWeapon::StartFire()
{
	if (isFiring) {
		return;
	}

	if (isReloading) {
		return;
	}

	isFiring = true;

	MeshComp->SetHiddenInGame(false, true);
}

void AThrowableWeapon::OnReload()
{
	Super::OnReload();

	if (CurrentAmmo > 0) {
		MeshComp->SetHiddenInGame(false, true);
	}
}
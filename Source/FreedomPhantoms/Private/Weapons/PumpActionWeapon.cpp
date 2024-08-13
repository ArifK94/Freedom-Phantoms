// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/PumpActionWeapon.h"

#include "Weapons//Weapon.h"
#include "Weapons//WeaponClip.h"

#include "FreedomPhantoms/FreedomPhantoms.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"

#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/AudioComponent.h"
#include "Math/Vector.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"

#include "TimerManager.h"

APumpActionWeapon::APumpActionWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	weaponType = WeaponType::Shotgun;

	HasLoadedShell = false;
	IsPullingPump = false;
	PumpActionBySound = false;
}

void APumpActionWeapon::BeginPlay()
{
	Super::BeginPlay();

	HasLoadedShell = true;
}

void APumpActionWeapon::Fire()
{
	if (CurrentAmmo <= 0) {
		isFiring = false;

		OnEmptyAmmoClip.Broadcast(this);

		if (!HasNoReload)
		{
			return;
		}
	}

	if (!HasLoadedShell) {
		return;
	}

	EndLoadShell();

	HasLoadedShell = false;

	Super::Fire();

	BeginLoadShell();
}


void APumpActionWeapon::BeginLoadShell()
{
	StopFire();

	// pull the handguard
	IsPullingPump = true;

	PlayPumpPullSound();

	GetWorldTimerManager().SetTimer(THandler_Pump, this, &APumpActionWeapon::EndLoadShell, PumpPullSound->GetDuration(), false);
}

void APumpActionWeapon::EndLoadShell()
{
	GetWorldTimerManager().ClearTimer(THandler_Pump);

	PlayPumpPushSound();

	BeginShellEffect();

	IsPullingPump = false;
	HasLoadedShell = true;
}


void APumpActionWeapon::OnReload()
{
	if (CurrentMaxAmmo <= 0 || CurrentAmmo >= AmmoPerClip) {
		EndLoadShell();
		return;
	}

	if (CurrentAmmo < AmmoPerClip)
	{
		isReloading = true;
		HasLoadedShell = true;

		CurrentAmmo++;

		if (InsertAmmoSound != NULL)
		{
			ClipAudioComponent->Sound = InsertAmmoSound;
			ClipAudioComponent->Play();
		}

		if (CurrentMaxAmmo > 0)
		{
			CurrentMaxAmmo--;
		}
		else
		{
			EndLoadShell();
		}
	}
	else
	{
		EndLoadShell();
	}
}

void APumpActionWeapon::PlayPumpPullSound()
{
	if (ClipAudioComponent != NULL)
	{
		if (ClipAudioComponent->IsPlaying())
			ClipAudioComponent->Stop();

		ClipAudioComponent->Sound = PumpPullSound;

		if (!ClipAudioComponent->IsPlaying())
			ClipAudioComponent->Play();
	}
}

void APumpActionWeapon::PlayPumpPushSound()
{
	if (ClipAudioComponent != NULL)
	{
		if (ClipAudioComponent->IsPlaying())
			ClipAudioComponent->Stop();


		ClipAudioComponent->Sound = PumpPushSound;

		if (!ClipAudioComponent->IsPlaying())
			ClipAudioComponent->Play();
	}
}

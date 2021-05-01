// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Shotgun.h"

#include "Weapons//Weapon.h"
#include "Weapons//WeaponClip.h"

#include "FreedomFighters/FreedomFighters.h"

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

AShotgun::AShotgun()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	PumpAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PumpAudioComponent"));

	weaponType = WeaponType::Shotgun;

	HasLoadedShell = false;
	IsPullingPump = false;
	PumpActionBySound = false;
}

void AShotgun::PlayHandguardPullSound()
{
	if (HandguardPullSound != NULL)
	{
		if (PumpAudioComponent->IsPlaying())
			PumpAudioComponent->Stop();

		PumpAudioComponent->Sound = HandguardPullSound;

		if (!PumpAudioComponent->IsPlaying())
			PumpAudioComponent->Play(0.0f);
	}
}

void AShotgun::PlayHandguardPushSound()
{
	if (HandguardPushSound != NULL)
	{
		if (PumpAudioComponent->IsPlaying())
			PumpAudioComponent->Stop();


		PumpAudioComponent->Sound = HandguardPushSound;

		if (!PumpAudioComponent->IsPlaying())
			PumpAudioComponent->Play(0.0f);
	}
}

void AShotgun::BeginPlay()
{
	Super::BeginPlay();

	HasLoadedShell = true;
}

void AShotgun::Fire()
{
	if (CurrentAmmo <= 0 || !HasLoadedShell) return;

	HasLoadedShell = false;
	isFiring = true;

	CurrentAmmo -= 1;

	CreateBullet();

	GetWorldTimerManager().SetTimer(THandler_PullPump, this, &AShotgun::beginHandguardPull, .3f, false);
}


void AShotgun::PullPump()
{
	if (!HasLoadedShell)
	{
		BeginShellEffect();
	}

	GetWorldTimerManager().ClearTimer(THandler_PullPump);

}


void AShotgun::PushPump()
{
	IsPullingPump = false;
	HasLoadedShell = true;
	isReloading = false;
}

void AShotgun::BeginHandguardTransition()
{
	beginHandguardPull();

	BeginShellEffect();

	FTimerHandle tHandle;
	const float Delay = .5f;
	GetWorldTimerManager().SetTimer(tHandle, this, &AShotgun::PushPump, Delay, false);
}

void AShotgun::beginHandguardPull()
{
	// pull the handguard
	IsPullingPump = true;

	PlayHandguardPullSound();
}

void AShotgun::OnReload()
{
	if (CurrentMaxAmmo <= 0 || CurrentAmmo >= AmmoPerClip) return;

	if (CurrentAmmo < AmmoPerClip)
	{
		isFiring = false;
		isReloading = true;
		HasLoadedShell = true;

		CurrentAmmo++;

		if (InsertAmmoSound != NULL)
		{
			PumpAudioComponent->Sound = InsertAmmoSound;
			PumpAudioComponent->Play(0.0f);
		}

		if (CurrentMaxAmmo > 0)
		{
			CurrentMaxAmmo--;
		}
	}
	else
	{
		isReloading = false;
	}
}
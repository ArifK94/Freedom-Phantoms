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

	HandguardAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("HandguardAudioComponent"));

	weaponType = WeaponType::Shotgun;

	Ammo_Holder = nullptr;
	hasLoadedShell = false;
	isPullingHandguard = false;

	HandguardSpeed = 0.0f;
}

void AShotgun::PlayHandguardPullSound()
{
	if (HandguardPullSound != NULL)
	{
		if (HandguardAudioComponent->IsPlaying())
			HandguardAudioComponent->Stop();

		HandguardAudioComponent->Sound = HandguardPullSound;

		if (!HandguardAudioComponent->IsPlaying())
			HandguardAudioComponent->Play(0.0f);
	}
}

void AShotgun::PlayHandguardPushSound()
{
	if (HandguardPushSound != NULL)
	{
		if (HandguardAudioComponent->IsPlaying())
			HandguardAudioComponent->Stop();


		HandguardAudioComponent->Sound = HandguardPushSound;

		if (!HandguardAudioComponent->IsPlaying())
			HandguardAudioComponent->Play(0.0f);
	}
}

void AShotgun::BeginPlay()
{
	Super::BeginPlay();

	setHandguard();

	hasLoadedShell = true;
}

void AShotgun::Fire()
{
	if (CurrentAmmo <= 0 || !hasLoadedShell) return;

	hasLoadedShell = false;
	isFiring = true;

	CurrentAmmo -= 1;

	CreateBullet();

	GetWorldTimerManager().SetTimer(pullHandguardTimeHandle, this, &AShotgun::beginHandguardPull, .3f, false);
}




void AShotgun::setHandguard()
{
	for (UActorComponent* component : GetComponentsByTag(UStaticMeshComponent::StaticClass(), "Handguard"))
	{
		HandguardComp = Cast<UStaticMeshComponent>(component);
	}

	if (HandguardComp)
	{
		HandguardMesh = HandguardComp;
	}
}



void AShotgun::pullHanguard()
{
	if (HandguardComp)
	{
		if (!hasLoadedShell)
		{
			BeginShellEffect();
		}

		GetWorldTimerManager().ClearTimer(pullHandguardTimeHandle);
	}
}


void AShotgun::pushHanguard()
{
	if (HandguardComp)
	{
		isPullingHandguard = false;
		hasLoadedShell = true;
		isReloading = false;
	}
}

void AShotgun::BeginHandguardTransition()
{
	beginHandguardPull();

	BeginShellEffect();

	FTimerHandle tHandle;
	const float Delay = .5f;
	GetWorldTimerManager().SetTimer(tHandle, this, &AShotgun::pushHanguard, Delay, false);
}

void AShotgun::beginHandguardPull()
{
	// pull the handguard
	isPullingHandguard = true;

	PlayHandguardPullSound();
}

void AShotgun::OnReload()
{
	if (MaxAmmo <= 0 || CurrentAmmo >= AmmoPerClip) return;

	if (CurrentAmmo < AmmoPerClip)
	{
		isFiring = false;
		isReloading = true;

		CurrentAmmo++;

		if (InsertAmmoSound != NULL)
		{
			HandguardAudioComponent->Sound = InsertAmmoSound;
			HandguardAudioComponent->Play(0.0f);
		}

		if (MaxAmmo > 0)
		{
			MaxAmmo--;
		}
	}
	else
	{
		isReloading = false;
	}
}
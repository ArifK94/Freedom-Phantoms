// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GrenadeLauncher.h"
#include "Weapons/WeaponClip.h"
#include "Weapons/Projectile.h"
#include "FreedomPhantoms/FreedomPhantoms.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "Particles/ParticleSystem.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/ArrowComponent.h"

#include "TimerManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundBase.h"
#include "Engine.h"


AGrenadeLauncher::AGrenadeLauncher()
{

}


void AGrenadeLauncher::Fire()
{
	if (CurrentAmmo <= 0) return;

	CurrentAmmo -= 1;

	isFiring = true;

	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{

		FVector EyeLocation;
		FRotator EyeRotation;

		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocket);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = MyOwner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		UWorld* world = GetWorld();
		auto bulletObj = world->SpawnActor<AProjectile>(weaponClipObj->getBulletClass(), MuzzleLocation, EyeRotation, SpawnParams);

		BeginShellEffect();

		LastFireTime = GetWorld()->TimeSeconds;

		// try and play the sound if specified
		if (ShotSound != NULL)
		{
			ShotAudioComponent->Sound = ShotSound;
			ShotAudioComponent->Play(0.0f);
		}

		if (CurrentAmmo <= 0)
		{
			BeginReload();
		}
	}
}
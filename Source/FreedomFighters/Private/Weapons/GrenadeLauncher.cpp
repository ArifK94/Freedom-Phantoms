// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GrenadeLauncher.h"
#include "Weapons/WeaponClip.h"
#include "Weapons/WeaponBullet.h"
#include "Managers/GameInstanceController.h"
#include "FreedomFighters/FreedomFighters.h"

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

	// Trace world from pawn eyes to cross hair location

	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{
		UWorld* world = GetWorld();

		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);


		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocket);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AWeaponBullet* bulletObj = world->SpawnActor<AWeaponBullet>(weaponClipObj->getBulletClass(), MuzzleLocation, FRotator::ZeroRotator, SpawnParams);

		//FVector ShotDirection = EyeRotation.Vector();
		//FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		//FCollisionQueryParams QueryParams;
		//QueryParams.AddIgnoredActor(MyOwner);
		//QueryParams.AddIgnoredActor(this);
		//QueryParams.bTraceComplex = true;
		//QueryParams.bReturnPhysicalMaterial = true;

		//// Particle "Target" parameter
		//FVector TracerEndPoint = TraceEnd;


		//FHitResult Hit;
		//if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		//{
		//	// Blocking hit! Process damage
		//	AActor* HitActor = Hit.GetActor();

		//	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

		//	float ActualDamage = BaseDamage;
		//	if (SurfaceType == SURFACE_FLESHVULNERABLE)
		//	{
		//		ActualDamage *= 4.0f;
		//	}

		//	UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

		//	UParticleSystem* SelectedEffect = gameInstanceController->CheckSurface(SurfaceType);

		//	if (SelectedEffect)
		//	{
		//		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		//	}

		//	TracerEndPoint = Hit.ImpactPoint;
		//}


		//BeginFireEffect(TracerEndPoint);
		//BeginShellEffect();


		Recoil();

		LastFireTime = GetWorld()->TimeSeconds;

		// try and play the sound if specified
		if (ShotSound != NULL)
		{
			ShotAudioComponent->Sound = ShotSound;
			ShotAudioComponent->Play(0.0f);
		}

		if (ShotEchoSound != NULL)
		{
			EchoAudioComponent->Sound = ShotEchoSound;
			EchoAudioComponent->Play(ShotSound->GetDuration());
		}

	}
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Shotgun.h"

#include "Weapons//Weapon.h"
#include "Weapons//WeaponClip.h"
#include "Managers/GameInstanceController.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/AudioComponent.h"
#include "Math/Vector.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"


AShotgun::AShotgun()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	HandguardAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("HandguardAudioComponent"));

	weaponType = WeaponType::Shotgun;

	Ammo_Holder = nullptr;
	hasLoadedShell = false;
	isPullingHandguard = false;
	useHanguardAnimation = false;

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

	setAmmoHolder();
	setHandguard();

	hasLoadedShell = true;

	if (HandguardPullSound != NULL)
	{
		pullDuration = HandguardPullSound->Duration;
	}
	else
	{
		pullDuration = 1.0f;
	}
}

void AShotgun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // Call parent class tick function  

	accumulatorFloat += DeltaTime;

	if (isPullingHandguard && !useHanguardAnimation)
	{
		pullHanguard();
	}

}

void AShotgun::Fire()
{
	if (CurrentAmmo <= 0) return;


	// Trace world from pawn eyes to cross hair location

	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{
		if (hasLoadedShell)
		{
			hasLoadedShell = false;

			CurrentAmmo -= 1;

			isFiring = true;

			FVector EyeLocation;
			FRotator EyeRotation;
			MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

			FVector ShotDirection = EyeRotation.Vector();
			FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MyOwner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			// Particle "Target" parameter
			FVector TracerEndPoint = TraceEnd;


			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
			{
				// Blocking hit! Process damage
				AActor* HitActor = Hit.GetActor();

				UGameplayStatics::ApplyPointDamage(HitActor, 1.0f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

				EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
				UParticleSystem* SelectedEffect = gameInstanceController->CheckSurface(SurfaceType);

				if (SelectedEffect)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
				}

				TracerEndPoint = Hit.ImpactPoint;
			}

			BeginFireEffect(TracerEndPoint);

			Recoil();

			LastFireTime = GetWorld()->TimeSeconds;

			if (ShotSound != NULL)
			{
				ShotAudioComponent->Sound = ShotSound;
				ShotAudioComponent->Play(0.0f);
			}

			GetWorldTimerManager().SetTimer(pullHandguardTimeHandle, this, &AShotgun::beginHandguardPull, .3f, false);
		}
	}
}


void AShotgun::setAmmoHolder()
{
	// Get Barrel Component
	for (UActorComponent* component : GetComponentsByTag(UStaticMeshComponent::StaticClass(), "AmmoHolder"))
	{
		Ammo_Holder = Cast<UStaticMeshComponent>(component);
	}

	// If there is a barrel component, set the mesh
	if (Ammo_Holder)
	{
		SpawnMagazine();
	}
}



void AShotgun::SpawnMagazine()
{
	UWorld* world = GetWorld();
	TArray<FName> allCartridges = Ammo_Holder->GetAllSocketNames();

	if (world)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		for (int i = 0; i < allCartridges.Num(); i++)
		{
			FString currentCartridge = allCartridges[i].ToString();

			if (currentCartridge.Contains("cartridge"))
			{
				// convert this to FName to set the socket
				FName targetSocket = FName(*currentCartridge);

				// Spawn the weapon actor
				weaponClipObj = world->SpawnActor<AWeaponClip>(weaponClip, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

				if (weaponClipObj)
				{
					weaponClipObj->SetOwner(this);
					weaponClipObj->AttachToComponent(Ammo_Holder, FAttachmentTransformRules::SnapToTargetNotIncludingScale, targetSocket);
				}
			}
		}
	}
}


void AShotgun::setHandguard()
{
	for (UActorComponent* component : GetComponentsByTag(UStaticMeshComponent::StaticClass(), "Handguard"))
	{
		HandguardComp = Cast<UStaticMeshComponent>(component);
	}

	if (HandguardComp)
	{
		//HandguardOffset = HandguardComp->GetSocketTransform("HandOffset",  ERelativeTransformSpace::RTS_World);
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

			if (!useHanguardAnimation)
			{
				// pull handguard
				FVector handguardTarget = UKismetMathLibrary::VInterpTo(HandguardOriginPos, HandguardPullPos, accumulatorFloat, HandguardSpeed);
				HandguardComp->SetRelativeLocation(handguardTarget);

				// if pull reached position
				if (handguardTarget == HandguardPullPos)
				{
					// wait for delay before loading again, for realism
					FTimerHandle tHandle;
					const float Delay = .5f;
					GetWorldTimerManager().SetTimer(tHandle, this, &AShotgun::pushHanguard, Delay, false);

					isPullingHandguard = false;
				}
			}
		}

		GetWorldTimerManager().ClearTimer(pullHandguardTimeHandle);
	}
}


void AShotgun::pushHanguard()
{
	if (HandguardComp)
	{
		if (!useHanguardAnimation)
		{
			FVector handguardLoadTarget = UKismetMathLibrary::VInterpTo(HandguardPullPos, HandguardOriginPos, accumulatorFloat, HandguardSpeed);
			HandguardComp->SetRelativeLocation(handguardLoadTarget);

			PlayHandguardPushSound();
		}

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
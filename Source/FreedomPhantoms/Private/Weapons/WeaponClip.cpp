// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponClip.h"
#include "Weapons/Projectile.h"

#include "Kismet/GameplayStatics.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

AWeaponClip::AWeaponClip()
{
	PrimaryActorTick.bCanEverTick = false;

	clipMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClipMeshComp"));
	clipMeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	clipMeshComp->CanCharacterStepUpOn = ECB_No;
	RootComponent = clipMeshComp;

	ammoCapacity = 30;

	CurrentAmmo = ammoCapacity;
}

void AWeaponClip::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = ammoCapacity;
}

void AWeaponClip::SetCurrentAmmo(int value)
{
	CurrentAmmo = value;
}


void AWeaponClip::DropClip(USkeletalMeshComponent* MeshComp, FName ClipSocket, TSubclassOf<class AWeaponClip> weaponClip)
{
	UWorld* world = GetWorld();

	if (world)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		FVector location = MeshComp->GetSocketLocation(ClipSocket);
		FRotator rotation = MeshComp->GetSocketRotation(ClipSocket);

		// Spawn the weapon actor
		DroppedClip = world->SpawnActor<AWeaponClip>(weaponClip, location, rotation, SpawnParams);

		if (DroppedClip)
		{
			DroppedClip->SetOwner(this);

			DroppedClip->getClipMesh()->SetSimulatePhysics(true);
			DroppedClip->getClipMesh()->SetEnableGravity(true);
			DroppedClip->getClipMesh()->SetNotifyRigidBodyCollision(true);
			DroppedClip->getClipMesh()->SetCollisionProfileName("Clip");
			DroppedClip->getClipMesh()->OnComponentHit.AddDynamic(this, &AWeaponClip::OnClipHit);
			DroppedClip->SetLifeSpan(10);
		}
	}
}

void AWeaponClip::OnClipHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		if (NormalImpulse.Z > 50)
		{
			if (HighImpactSound != NULL)
			{
				if (CollisionAudioComponent == nullptr) {
					CollisionAudioComponent = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), HighImpactSound, DroppedClip->getClipMesh()->GetComponentLocation(), FRotator::ZeroRotator,  .5f, 1.f, 0.f, ClipAttenuationSettings);
				}
				else {
					CollisionAudioComponent->Play();
				}
			}
		}

	}
}

void AWeaponClip::SpawnBullet()
{
	UWorld* world = GetWorld();

	if (WeaponBulletClass)
	{
		if (world)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// Spawn the weapon actor
			BulletObj = world->SpawnActor<AProjectile>(WeaponBulletClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (BulletObj)
			{
				BulletObj->SetOwner(this);
				BulletObj->getMesh()->ToggleVisibility(false);
			}
		}
	}
}

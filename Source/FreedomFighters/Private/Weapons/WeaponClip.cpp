// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponClip.h"
#include "Weapons/Projectile.h"

#include "Kismet/GameplayStatics.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

AWeaponClip::AWeaponClip()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	clipMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClipMeshComp"));
	clipMeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	clipMeshComp->CanCharacterStepUpOn = ECB_No;

	ammoCapacity = 30;

	CurrentAmmo = ammoCapacity;
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
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HighImpactSound, DroppedClip->getClipMesh()->GetComponentLocation(), .5f, 1.f, 0.f, ClipAttenuationSettings);
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
			BulletObj = world->SpawnActor<AWeaponBullet>(WeaponBulletClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (BulletObj)
			{
				BulletObj->SetOwner(this);
				BulletObj->getMesh()->ToggleVisibility(false);
			}
		}
	}
}

// Called when the game starts or when spawned
void AWeaponClip::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = ammoCapacity;
}

// Called every frame
void AWeaponClip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponClip::SetCurrentAmmo(int value)
{
	CurrentAmmo = value;
}


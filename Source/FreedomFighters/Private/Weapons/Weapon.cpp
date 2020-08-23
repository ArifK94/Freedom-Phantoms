#include "Weapons/Weapon.h"
#include "Weapons/WeaponClip.h"
#include "Weapons/WeaponBullet.h"
#include "Weapons/WeaponAttachmentManager.h"

#include "FreedomFighters/FreedomFighters.h"
#include "Managers/GameInstanceController.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/Vector.h"

#include "Particles/ParticleSystem.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SkinnedMeshComponent.h"

#include "TimerManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundBase.h"
#include "Engine.h"
#include "UObject/UObjectGlobals.h"


// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComp->CanCharacterStepUpOn = ECB_No;

	ShotAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ShotAudioComponent"));
	ClipAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ClipAudioComponent"));

	MuzzleSocket = "Muzzle";
	TracerTargetSocket = "Target";
	ClipSocket = "Clip";
	HolsterSocket = "holster1";
	EjectorSocket = "Ejector";
	HandguardSocket = "HandguardSocket";
	ReloadClipHandSocket = "clip_hand";
	OpticsSocket = "Optics";
	LaserSocket = "Laser";
	TorchlightSocket = "Torchlight";
	ParentHolderSocket = "Hand";

	MaxAmmo = 120;
	RateOfFire = 600.0f;
	BulletDamage = 20.0f;

	RecoilAmount = 5.0f;
	VerticleRecoil = 0.05f;
	HorizontalRecoil = 0.01f;
	BulletSpread = 2.0f;

	isReloading = false;
	canShowClip = true;
	canAutoReload = true;


}

void AWeapon::ConfigSetup()
{
	HandguardMesh = MeshComp;

	AmmoPerClip = weaponClipObj->GetAmmoCapacity();
	CurrentAmmo = AmmoPerClip;

	TimeBetweenShots = 60 / RateOfFire;
}

FVector AWeapon::getMuzzleLocation()
{
	return MeshComp->GetSocketLocation(MuzzleSocket);
}


void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	UWorld* world = GetWorld();
	UGameInstance* instance = UGameplayStatics::GetGameInstance(world);
	gameInstanceController = Cast<UGameInstanceController>(instance);

	SpawnMagazine();
	ConfigSetup();
	SpawnWeaponAttachments();

}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentDeltaTime = DeltaTime;

	if (weaponClipObj)
	{
		weaponClipObj->SetCurrentAmmo(CurrentAmmo);
	}

	if (CurrentAmmo <= 0)
		isFiring = false;

	AmmoCount = FString::FromInt(CurrentAmmo) + " / " + FString::FromInt(MaxAmmo);

	CurrentMuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocket);
	CurrentMuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocket);


	// Reset Burst fire count
	if (selectiveFireMode == SelectiveFire::Burst)
	{
		if (BurstAmmountCount >= 3)
		{
			StopFire();
		}
	}
	else if (selectiveFireMode == SelectiveFire::SemiAutomatic)
	{
		if (BurstAmmountCount >= 1)
		{
			StopFire();
		}
	}
}

void AWeapon::Fire()
{
	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{

		if (CurrentAmmo <= 0)
		{
			if (canAutoReload)
			{
				BeginReload();
				return;
			}
			else
			{
				return;
			}
		}

		// Trace world from pawn eyes to cross hair location
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		isFiring = true;

		if (IsFacingCrosshair())
		{
			CurrentAmmo -= 1;

			FVector ShotDirection = EyeRotation.Vector();

			float HalfRad = FMath::DegreesToRadians(BulletSpread);
			ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

			FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MyOwner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			// Particle "Target" parameter
			FVector TracerEndPoint = TraceEnd;


			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
			{
				// Blocking hit! Process damage
				AActor* HitActor = Hit.GetActor();

				EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

				float ActualDamage = BulletDamage;
				if (SurfaceType == SURFACE_FLESHVULNERABLE)
				{
					ActualDamage *= 100;
				}

				UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

				UParticleSystem* SelectedEffect = gameInstanceController->CheckSurface(SurfaceType);

				if (SelectedEffect)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
				}

				TracerEndPoint = Hit.ImpactPoint;
			}

			PlayShotEffect(TracerEndPoint);

			BurstAmmountCount++;
		}
	}
}

void AWeapon::BurstDelay()
{
	if (BurstAmmountCount < 3)
	{
		GetWorldTimerManager().SetTimer(THandler_TimeBetweenShots, this, &AWeapon::Fire, TimeBetweenShots, true, 0.0f);
	}
}

void AWeapon::SemiFireDelay()
{
	if (BurstAmmountCount < 1)
	{
		GetWorldTimerManager().SetTimer(THandler_TimeBetweenShots, this, &AWeapon::Fire, TimeBetweenShots, true, 0.0f);
	}
}


void AWeapon::StartFire()
{
	if (!isReloading)
	{
		isFiring = true;

		float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

		switch (selectiveFireMode)
		{
		case SelectiveFire::Automatic:
			GetWorldTimerManager().SetTimer(THandler_TimeBetweenShots, this, &AWeapon::Fire, TimeBetweenShots, true, FirstDelay);
			break;
		case SelectiveFire::SemiAutomatic:
			GetWorldTimerManager().SetTimer(THandler_TimeBetweenShots, this, &AWeapon::SemiFireDelay, TimeBetweenShots, true, FirstDelay);
			break;
		case SelectiveFire::Burst:
			GetWorldTimerManager().SetTimer(THandler_TimeBetweenShots, this, &AWeapon::BurstDelay, TimeBetweenShots, true, FirstDelay);
			break;
		default:
			Fire();
			break;
		}
	}
}




void AWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(THandler_TimeBetweenShots);

	isFiring = false;

	CurrentVerticleRecoil = 0.0f;
	BurstAmmountCount = 0;

}

bool AWeapon::IsFacingCrosshair()
{
	return true;

	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		// Dot product allows to check if muzzle is facing in same direction as the camera view
		float directionValue = FVector::DotProduct(UKismetMathLibrary::GetForwardVector(EyeRotation), UKismetMathLibrary::GetForwardVector(CurrentMuzzleRotation));

		FVector locationValue = UKismetMathLibrary::Subtract_VectorVector(CurrentMuzzleLocation, EyeLocation);

		float limit = 50.0f;
		if (UKismetMathLibrary::Abs(directionValue) <= 0.5f && (UKismetMathLibrary::Abs(locationValue.X) <= limit || UKismetMathLibrary::Abs(locationValue.Y) <= limit || UKismetMathLibrary::Abs(locationValue.Z) <= limit))
		{
			return true;
		}
	}

	return false;
}


void AWeapon::PlayShotEffect(FVector TracerEndPoint)
{
	BeginFireEffect(TracerEndPoint);
	BeginShellEffect();


	Recoil();

	LastFireTime = GetWorld()->TimeSeconds;

	// try and play the sound if specified
	if (ShotSound != NULL)
	{
		ShotAudioComponent->Sound = ShotSound;
		ShotAudioComponent->Play(0.0f);
	}
}


void AWeapon::BeginFireEffect(FVector TraceEnd)
{
	if (gameInstanceController->MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), gameInstanceController->MuzzleEffect, getMuzzleLocation());
	}

	if (gameInstanceController->TracerEffect)
	{
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), gameInstanceController->TracerEffect, getMuzzleLocation());

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetSocket, TraceEnd);
		}
	}


	CameraShakeEffect();

}

void AWeapon::BeginShellEffect()
{
	if (ShellEjectEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShellEjectEffect, MeshComp->GetSocketLocation(EjectorSocket));
	}
}

void AWeapon::CameraShakeEffect()
{
	APawn* MyOwner = Cast<APawn>(GetOwner());

	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());

		if (PC)
		{
			PC->ClientPlayCameraShake(gameInstanceController->FireCamShake);
		}
	}
}

void AWeapon::OnReload()
{
	// Do we have ammo in the ammopool?
	if (MaxAmmo <= 0 || CurrentAmmo >= AmmoPerClip)	return;

	// Do we have enough to meet what the weapon needs?
	if (MaxAmmo < (AmmoPerClip - CurrentAmmo))
	{
		CurrentAmmo = CurrentAmmo + MaxAmmo;
		MaxAmmo = 0;
	}
	else
	{
		MaxAmmo = MaxAmmo - (AmmoPerClip - CurrentAmmo);
		CurrentAmmo = AmmoPerClip;
	}
}

void AWeapon::BeginReload()
{
	if (MaxAmmo <= 0 || CurrentAmmo >= AmmoPerClip && !isReloading)	return;

	isFiring = false;
	isReloading = true;

	weaponClipObj->getClipMesh()->SetVisibility(false);

}

void AWeapon::EndReload()
{
	if (ReloadEndSound != NULL)
	{
		ClipAudioComponent->Sound = ReloadEndSound;
		ClipAudioComponent->Play(0.0f);
	}

	isReloading = false;
}

void AWeapon::ClipIn()
{
	if (ReloadClipInSound != NULL)
	{
		ClipAudioComponent->Sound = ReloadClipInSound;
		ClipAudioComponent->Play(0.0f);
	}

	OnReload();
	SetMagazineSocket();
}

void AWeapon::ClipOut()
{
	if (ReloadClipOutSound != NULL)
	{
		ClipAudioComponent->Sound = ReloadClipOutSound;
		ClipAudioComponent->Play(0.0f);
	}

	// Drop magazine
	if (weaponClip && canShowClip)
	{
		weaponClipObj->DropClip(MeshComp, ClipSocket, weaponClip);
	}
}

void AWeapon::SpawnMagazine()
{
	UWorld* world = GetWorld();

	if (weaponClip)
	{
		if (world)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// Spawn the weapon actor
			weaponClipObj = world->SpawnActor<AWeaponClip>(weaponClip, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (weaponClipObj)
			{
				weaponClipObj->SetOwner(this);
				SetMagazineSocket();

				// Some weapons may not show clip such as certain shotguns
				if (!canShowClip)
				{
					weaponClipObj->getClipMesh()->ToggleVisibility(false);
				}
			}
		}
	}
}

void AWeapon::Recoil()
{
	AActor* MyOwner = GetOwner();

	auto character = UGameplayStatics::GetPlayerController(MyOwner, 0);
	VerticleRecoil = VerticleRecoil * -1;

	CurrentVerticleRecoil += .2f;

	TargetHorizontalRecoil = UKismetMathLibrary::RandomFloatInRange(-0.3, 0.3);
	TargetVerticalRecoil = UKismetMathLibrary::RandomFloatInRange(-0.5f, -0.8f) * 2.0f / 3.0f;

	//character->AddPitchInput(TargetVerticalRecoil);
	//character->AddYawInput(TargetHorizontalRecoil);
}




void AWeapon::SetMagazineSocket()
{
	weaponClipObj->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ClipSocket);
}

void AWeapon::SetClipSocket(USkeletalMeshComponent* meshComponent)
{
	weaponClipObj->getClipMesh()->SetVisibility(true);
	weaponClipObj->AttachToComponent(meshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ReloadClipHandSocket);
}



void AWeapon::setWeaponSocket(USkeletalMeshComponent* meshComponent, FName socket)
{
	MeshComp->AttachToComponent(meshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, socket);
}

void AWeapon::SpawnWeaponAttachments()
{
	UWorld* World = GetWorld();

	if (World && WeaponAttachmentClass != NULL)
	{
		WeaponAttachmentObj = NewObject<UWeaponAttachmentManager>((UObject*)GetTransientPackage(), WeaponAttachmentClass);
		WeaponAttachmentObj->SpawnAttachments(MeshComp, this, World);

		if (WeaponAttachmentObj)
		{
			if (WeaponAttachmentObj->getUnderBarrelWeaponObj())
			{
				HandguardMesh = WeaponAttachmentObj->getUnderBarrelWeaponObj()->getMeshComp();
			}
		}

	}
}

void AWeapon::SetHandGuardIK(USkeletalMeshComponent* CharacterMesh)
{
	FVector TargetPosition;
	FRotator TargetRotation;

	FTransform InputTransform = HandguardMesh->GetSocketTransform(HandguardSocket, RTS_World);
	CharacterMesh->TransformToBoneSpace("j_wrist_ri", InputTransform.GetLocation(), InputTransform.GetRotation().Rotator(), TargetPosition, TargetRotation);
	HandguardOffset.SetLocation(TargetPosition);
	HandguardOffset.SetRotation(TargetRotation.Quaternion());
}


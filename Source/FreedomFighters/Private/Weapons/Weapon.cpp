#include "Weapons/Weapon.h"
#include "Weapons/WeaponClip.h"
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
	WeaponHandSocket = "weapon_hand";
	ReloadClipHandSocket = "clip_hand";


	MaxAmmo = 120;
	RateOfFire = 600.0f;
	BaseDamage = 20.0f;

	RecoilAmount = 5.0f;
	VerticleRecoil = 0.05f;
	HorizontalRecoil = 0.01f;

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


	MuzzleLocationTest = MeshComp->GetSocketLocation(MuzzleSocket);
	MuzzleRotationTest = MeshComp->GetSocketRotation(MuzzleSocket);
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

		// Dot product allows to check if muzzle is facing in same direction as the camera view
		float directionValue = FVector::DotProduct(UKismetMathLibrary::GetForwardVector(EyeRotation), UKismetMathLibrary::GetForwardVector(MuzzleRotationTest));

		if (UKismetMathLibrary::Abs(directionValue) <= 0.5f)
		{
			CurrentAmmo -= 1;

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
			if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
			{
				// Blocking hit! Process damage
				AActor* HitActor = Hit.GetActor();

				EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

				float ActualDamage = BaseDamage;
				if (SurfaceType == SURFACE_FLESHVULNERABLE)
				{
					ActualDamage *= 4.0f;
				}

				UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

				UParticleSystem* SelectedEffect = gameInstanceController->CheckSurface(SurfaceType);

				if (SelectedEffect)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
				}

				TracerEndPoint = Hit.ImpactPoint;
			}

			//DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);

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
	}
}


void AWeapon::StartFire()
{
	if (!isReloading)
	{
		float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

		switch (selectiveFireMode)
		{
		case SelectiveFire::Automatic:
			GetWorldTimerManager().SetTimer(THandler_TimeBetweenShots, this, &AWeapon::Fire, TimeBetweenShots, true, FirstDelay);
			break;
		case SelectiveFire::SemiAutomatic:
			break;
		case SelectiveFire::Burst:
			break;
		case SelectiveFire::Single:
			Fire();
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
	if (CurrentAmmo < AmmoPerClip && MaxAmmo > 0)
	{
		isFiring = false;
		isReloading = true;

		weaponClipObj->getClipMesh()->SetVisibility(false);
	}
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


	TargetHorizontalRecoil = UKismetMathLibrary::RandomFloatInRange(-0.15, 0.15);
	TargetVerticalRecoil = UKismetMathLibrary::RandomFloatInRange(0.5, -0.5f);


	//	TargetVerticalRecoil = UKismetMathLibrary::Lerp(0.0f, VerticleRecoil, RecoilAmount);
	//	TargetHorizontalRecoil = UKismetMathLibrary::Lerp(0.0f, HorizontalRecoil, RecoilAmount);

	character->AddPitchInput(TargetVerticalRecoil);
	character->AddYawInput(TargetHorizontalRecoil);
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

int32 AWeapon::getCurrentAmmo()
{
	return CurrentAmmo;
}

int32 AWeapon::getMaxAmmo()
{
	return MaxAmmo;
}

int32 AWeapon::getAmmoPerClip()
{
	return AmmoPerClip;
}


void AWeapon::setWeaponSocket(USkeletalMeshComponent* meshComponent, FName socket)
{
	MeshComp->AttachToComponent(meshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, socket);
}

FName AWeapon::getHolsterSocket()
{
	return HolsterSocket;
}

FName AWeapon::getWeaponHandSocket()
{
	return WeaponHandSocket;
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

	FTransform InputTransform  = HandguardMesh->GetSocketTransform(HandguardSocket, RTS_World);
	CharacterMesh->TransformToBoneSpace("j_wrist_ri", InputTransform.GetLocation(), InputTransform.GetRotation().Rotator(), TargetPosition, TargetRotation);
	HandguardOffset.SetLocation(TargetPosition);
	HandguardOffset.SetRotation(TargetRotation.Quaternion());
}


#include "Weapons/Weapon.h"
#include "Weapons/WeaponClip.h"
#include "Weapons/WeaponBullet.h"
#include "Weapons/WeaponAttachmentManager.h"

#include "FreedomFighters/FreedomFighters.h"

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
#include "Components/SceneComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/ObjectPoolComponent.h"


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

	ObjectPoolComponent = CreateDefaultSubobject<UObjectPoolComponent>(TEXT("ObjectPoolComponent"));
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

	MaxAmmoCapacity = 120;
	RateOfFire = 600.0f;
	CooldownReload = 0.0f;

	RecoilAmount = 5.0f;
	VerticleRecoil = 0.05f;
	HorizontalRecoil = 0.01f;
	BulletSpread = 2.0f;
	ZoomFOV = 90.0f;

	isReloading = false;
	canShowClip = true;
	HasUnlimitedAmmo = false;
	hasRecoil = true;

	HasPlayedClipIn = false;
	HasPlayedClipOut = false;
}

void AWeapon::ConfigSetup()
{
	HandguardMesh = MeshComp;

	if (weaponClipObj)
	{
		AmmoPerClip = weaponClipObj->GetAmmoCapacity();
		CurrentAmmo = AmmoPerClip;
	}
	else
	{
		CurrentAmmo = AmmoPerClip;
	}

	AActor* MyOwner = GetOwner();
	// Add neccessary Actors to the Object pool
	if (BulletClass && MyOwner)
	{
		FObjectPoolParameters ObjectPoolParams;
		ObjectPoolParams.PoolSize = AmmoPerClip;
		ObjectPoolParams.LifeSpan = 5.0f;
		ObjectPoolParams.ActorClass = BulletClass;
		ObjectPoolComponent->AddToPool(MyOwner, ObjectPoolParams);
	}

	TimeBetweenShots = 60 / RateOfFire;

	CurrentMaxAmmo = MaxAmmoCapacity;
}

FVector AWeapon::getMuzzleLocation()
{
	return MeshComp->GetSocketLocation(MuzzleSocket);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	ShotAudioComponent->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MuzzleSocket);
	ClipAudioComponent->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MuzzleSocket);

	SpawnMagazine();
	ConfigSetup();
	SpawnWeaponAttachments();

	CurrentCountdownReload = CooldownReload;
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

	AutoReload();
}

void AWeapon::Fire()
{
	if (CurrentAmmo <= 0) return;

	isFiring = true;

	CurrentAmmo -= 1;

	CreateBullet();

	BurstAmmountCount++;
}

void AWeapon::CreateBullet()
{
	AActor* MyOwner = GetOwner();

	if (!MyOwner) return;

	// Trace world from pawn eyes to cross hair location
	if (EyeViewPointComponent)
	{
		EyeLocation = EyeViewPointComponent->GetComponentLocation();
		EyeRotation = EyeViewPointComponent->GetComponentRotation();
	}
	else
	{
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	}

	FVector ShotDirection = EyeRotation.Vector();

	if (hasRecoil)
	{
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
	}


	FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(MyOwner);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	// Particle "Target" parameter
	FVector TracerEndPoint = TraceEnd;

	FHitResult Hit;
	bool LineTraceFire = GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams);
	if (LineTraceFire)
	{
		TracerEndPoint = Hit.ImpactPoint;
	}

	if (BulletClass)
	{
		FObjectPoolParameters ObjectPoolParams;
		ObjectPoolParams.PoolSize = 1;
		ObjectPoolParams.LifeSpan = 5.0f;
		ObjectPoolParams.ActorClass = BulletClass;
		ObjectPoolComponent->ActivatePoolObject(BulletClass, getMuzzleLocation(), UKismetMathLibrary::FindLookAtRotation(getMuzzleLocation(), TracerEndPoint), MyOwner, ObjectPoolParams);
	}

	PlayShotEffect(TracerEndPoint);
}

void AWeapon::PlayShotEffect(FVector TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleEffect, getMuzzleLocation());
	}

	if (TracerEffect)
	{
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, getMuzzleLocation());

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetSocket, TracerEndPoint);
		}
	}

	Recoil();

	LastFireTime = GetWorld()->TimeSeconds;

	// try and play the sound if specified
	if (ShotSound != NULL)
	{
		ShotAudioComponent->Sound = ShotSound;
		ShotAudioComponent->Play(0.0f);
	}
}

void AWeapon::BeginShellEffect()
{
	if (ShellEjectEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShellEjectEffect, MeshComp->GetSocketLocation(EjectorSocket));
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


void AWeapon::OnReload()
{
	// Do we have ammo in the ammopool?
	if (CurrentMaxAmmo <= 0 || CurrentAmmo >= AmmoPerClip && !HasUnlimitedAmmo) return;

	if (HasUnlimitedAmmo)
	{
		CurrentAmmo = AmmoPerClip;
	}
	else
	{
		// Do we have enough to meet what the weapon needs?
		if (CurrentMaxAmmo < (AmmoPerClip - CurrentAmmo))
		{
			CurrentAmmo = CurrentAmmo + CurrentMaxAmmo;
			CurrentMaxAmmo = 0;
		}
		else
		{
			CurrentMaxAmmo = CurrentMaxAmmo - (AmmoPerClip - CurrentAmmo);

			CurrentAmmo = AmmoPerClip;
		}
	}
}

void AWeapon::BeginReload()
{
	if (CurrentMaxAmmo <= 0 || CurrentAmmo >= AmmoPerClip && !isReloading)	return;

	isFiring = false;
	isReloading = true;

	if (weaponClipObj) {
		weaponClipObj->getClipMesh()->SetVisibility(false);
	}

}

void AWeapon::EndReload()
{
	if (ReloadEndSound != NULL)
	{
		ClipAudioComponent->Sound = ReloadEndSound;
		ClipAudioComponent->Play();
	}

	isReloading = false;
	HasPlayedClipIn = false;
	HasPlayedClipOut = false;
}

void AWeapon::ClipIn()
{
	if (!HasPlayedClipIn && ReloadClipInSound != NULL)
	{
		ClipAudioComponent->Sound = ReloadClipInSound;
		ClipAudioComponent->Play();
		HasPlayedClipIn = true;
	}

	OnReload();
	SetMagazineSocket();
}

void AWeapon::ClipOut()
{

	if (!HasPlayedClipOut && ReloadClipOutSound != NULL)
	{
		ClipAudioComponent->Sound = ReloadClipOutSound;
		ClipAudioComponent->Play();
		HasPlayedClipOut = true;
	}


	// Drop magazine
	if (weaponClipObj && weaponClip && canShowClip)
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
	if (weaponClipObj) {
		weaponClipObj->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ClipSocket);
	}
}

void AWeapon::SetClipSocket(USkeletalMeshComponent* meshComponent)
{
	if (weaponClipObj) {
		weaponClipObj->getClipMesh()->SetVisibility(true);
		weaponClipObj->AttachToComponent(meshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ReloadClipHandSocket);
	}
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

void AWeapon::SetHandGuardIK(USkeletalMeshComponent* CharacterMesh, FName TriggerHandSocket)
{
	FVector TargetPosition;
	FRotator TargetRotation;

	FTransform InputTransform = HandguardMesh->GetSocketTransform(HandguardSocket, RTS_World);
	CharacterMesh->TransformToBoneSpace(TriggerHandSocket, InputTransform.GetLocation(), InputTransform.GetRotation().Rotator(), TargetPosition, TargetRotation);
	HandguardOffset.SetLocation(TargetPosition);
	HandguardOffset.SetRotation(TargetRotation.Quaternion());
}

void AWeapon::AutoReload()
{
	if (CooldownReload > 0.0f && CurrentAmmo <= 0.0f)
	{
		if (CurrentCountdownReload > 0.0f)
		{
			CurrentCountdownReload -= CurrentDeltaTime;

			isReloading = true;

			if (CurrentCountdownReload > (CooldownReload / 2)) // if countdown is half way through, change sound clip
			{
				ClipOut();
			}
			else
			{
				if (!HasPlayedClipIn && ReloadClipInSound != NULL)
				{
					ClipAudioComponent->Sound = ReloadClipInSound;
					ClipAudioComponent->Play();
					HasPlayedClipIn = true;
				}
			}
		}
		else
		{
			OnReload();
			EndReload();
			CurrentCountdownReload = CooldownReload;
			isReloading = false;
		}
	}
}
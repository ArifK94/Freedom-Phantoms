#include "Weapons/Weapon.h"
#include "Weapons/WeaponClip.h"
#include "Weapons/WeaponBullet.h"
#include "Weapons/WeaponAttachmentManager.h"

#include "FreedomFighters/FreedomFighters.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "Math/Vector.h"

#include "Particles/ParticleSystem.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "CustomComponents/ObjectPoolComponent.h"


#include "TimerManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundBase.h"
#include "Engine.h"
#include "UObject/UObjectGlobals.h"


void AWeapon::SetIsAiming(bool isAiming)
{
	IsAiming = isAiming;
}

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
	ChargingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ChargingAudioComponent"));


	MuzzleSocket = "Muzzle";
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

	BulletSpreadRadius = 5.0f;
	ZoomFOV = 90.0f;
	BulletsPerFire = 1;

	isReloading = false;
	canShowClip = true;
	HasUnlimitedAmmo = false;
	hasRecoil = true;

	HasPlayedClipIn = false;
	HasPlayedClipOut = false;

	PlayFireSoundAtLocation = true;

	CanAutoReload = false;

	HasNoReload = false;

	ChargeSoundParamName = "ChargeAmount";

	DrawShotLine = false;
	ShotLineDuration = 5.0f;
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

	// Add neccessary Actors to the Object pool
	if (BulletClass)
	{
		FObjectPoolParameters* ObjectPoolParams = new FObjectPoolParameters();
		ObjectPoolParams->PoolableActorClass = BulletClass;

		if (AmmoPerClip <= 0)
		{
			// for weapons that are mounted which do not have a reload system
			ObjectPoolParams->PoolSize = 10;
		}
		else
		{
			ObjectPoolParams->PoolSize = AmmoPerClip;
		}

		ObjectPoolComponent->AddToPool(ObjectPoolParams);
	}

	TimeBetweenShots = 60 / RateOfFire;

	CurrentMaxAmmo = MaxAmmoCapacity;
	CurrentChargeUpTime = 0.0f;
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
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (weaponClipObj)
	{
		weaponClipObj->SetCurrentAmmo(CurrentAmmo);
	}

	if (CurrentAmmo <= 0)
		isFiring = false;


	CurrentMuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocket);
	CurrentMuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocket);
}

void AWeapon::Fire()
{
	if (isReloading) {
		return;
	}

	if (CurrentAmmo <= 0 && !HasNoReload) {
		return;
	}

	// Reset Burst fire count
	if (selectiveFireMode == SelectiveFire::Burst)
	{
		if (BurstAmmountCount >= 3)
		{
			StopFire();
			return;
		}
	}
	else if (selectiveFireMode == SelectiveFire::SemiAutomatic)
	{
		if (BurstAmmountCount >= 1)
		{
			StopFire();
			return;
		}
	}

	isFiring = true;

	if (!HasNoReload) {
		CurrentAmmo -= 1;
	}


	CreateBullet();

	BurstAmmountCount++;

	if (CanAutoReload && !HasNoReload)
	{
		if (CurrentAmmo <= 0)
		{
			isReloading = true;
			ClipOut();
			GetWorldTimerManager().SetTimer(THandler_AutoReloadBegin, this, &AWeapon::AutoReloadBegin, CooldownReload / 2.0f, false);
		}
	}
}

void AWeapon::CreateBullet()
{
	AActor* MyOwner = GetOwner();

	if (!MyOwner) {
		return;
	}


	float TraceLength = 10000.0f;

	FVector EyeLocation;
	FRotator EyeRotation;

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

	for (int i = 0; i < BulletsPerFire; i++)
	{
		FVector RandomRadius = RandomPointInCircle(UKismetMathLibrary::DegTan(BulletSpreadRadius) * TraceLength);

		FVector TraceEnd = EyeLocation + (ShotDirection * TraceLength);

		if (hasRecoil)
		{
			TraceEnd += (UKismetMathLibrary::GetRightVector(EyeRotation) * RandomRadius.X) + (UKismetMathLibrary::GetUpVector(EyeRotation) * RandomRadius.Y);
		}


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
			ObjectPoolComponent->ActivatePoolObject(BulletClass, MyOwner, getMuzzleLocation(), UKismetMathLibrary::FindLookAtRotation(getMuzzleLocation(), TracerEndPoint));
		}

		if (DrawShotLine)
		{
			TArray<AActor*> ActorsToIgnore;
			UKismetSystemLibrary::LineTraceSingle(GetWorld(), EyeLocation, TraceEnd, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForDuration, Hit, true, FLinearColor::Blue, FLinearColor::Green, ShotLineDuration);
		}
	}

	PlayShotEffect(EyeLocation);
}

void AWeapon::PlayShotEffect(FVector EyeLocation)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleEffect, getMuzzleLocation());
	}


	LastFireTime = GetWorld()->TimeSeconds;

	// try and play the sound if specified
	if (ShotSound != nullptr)
	{
		if (PlayFireSoundAtLocation)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ShotSound, EyeLocation, 1.f, 1.f, 0.f, FireAttenuation);
		}
		else
		{
			ShotAudioComponent->Sound = ShotSound;
			ShotAudioComponent->Play(0.0f);
		}
	}
}

// Bullet spread random point
FVector AWeapon::RandomPointInCircle(float Radius)
{
	FVector Target;
	float Angle = UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f);
	float DistanceFromCenter = UKismetMathLibrary::RandomFloatInRange(0.0f, Radius);

	Target.X = DistanceFromCenter * UKismetMathLibrary::DegCos(Angle);
	Target.Y = DistanceFromCenter * UKismetMathLibrary::DegSin(Angle);

	return Target;
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

	ChargeUp();
}

void AWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(THandler_TimeBetweenShots);

	isFiring = false;

	CurrentVerticleRecoil = 0.0f;
	BurstAmmountCount = 0;

	ChargeDown();
}

void AWeapon::ChargeUp()
{
	if (IsChargingUp) {
		return;
	}

	GetWorldTimerManager().ClearTimer(THandler_ChargeDown);

	IsChargingUp = true;

	if (ChargeUpSound != nullptr)
	{
		ChargingAudioComponent->Sound = ChargeUpSound;
		ChargingAudioComponent->Play();
	}

	GetWorldTimerManager().SetTimer(THandler_ChargeUp, this, &AWeapon::IncreaseCharge, .1f, true);
}

void AWeapon::ChargeDown()
{
	if (isFiring) {
		return;
	}

	if (IsAiming) {
		return;
	}

	GetWorldTimerManager().ClearTimer(THandler_ChargeUp);
	IsChargingUp = false;


	if (ChargeDownSound != nullptr)
	{
		ChargingAudioComponent->Sound = ChargeDownSound;
		ChargingAudioComponent->Play();
	}

	GetWorldTimerManager().SetTimer(THandler_ChargeDown, this, &AWeapon::DecreaseCharge, .1f, true);

}

void AWeapon::IncreaseCharge()
{
	ChargingAudioComponent->SetFloatParameter(ChargeSoundParamName, CurrentChargeUpTime);

	if (CurrentChargeUpTime < ChargeUpTime)
	{
		CurrentChargeUpTime += .1f;
	}
	else
	{
		CurrentChargeUpTime = ChargeUpTime;
	}
}

void AWeapon::DecreaseCharge()
{
	ChargingAudioComponent->SetFloatParameter(ChargeSoundParamName, CurrentChargeUpTime);

	if (CurrentChargeUpTime > 0.0f)
	{
		CurrentChargeUpTime -= .1f;
	}
	else
	{
		CurrentChargeUpTime = 0.0f;

		GetWorldTimerManager().ClearTimer(THandler_ChargeDown);
	}
}

void AWeapon::OnReload()
{
	// Do we have ammo in the ammopool?
	if (CurrentMaxAmmo <= 0 || CurrentAmmo >= AmmoPerClip && !HasUnlimitedAmmo) {
		return;
	}

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
	if (!weaponClip) {
		return;
	}

	UWorld* world = GetWorld();

	if (!world) {
		return;
	}
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


void AWeapon::AutoReloadBegin()
{
	if (ReloadClipInSound != NULL)
	{
		ClipAudioComponent->Sound = ReloadClipInSound;
		ClipAudioComponent->Play();
	}

	GetWorldTimerManager().SetTimer(THandler_AutoReloadEnd, this, &AWeapon::AutoReloadEnd, CooldownReload / 2.0f, false);
	GetWorldTimerManager().ClearTimer(THandler_AutoReloadBegin);
}

void AWeapon::AutoReloadEnd()
{
	OnReload();
	EndReload();
	isReloading = false;
	GetWorldTimerManager().ClearTimer(THandler_AutoReloadEnd);
}


bool AWeapon::ReplenishAmmo()
{
	if (CurrentMaxAmmo < MaxAmmoCapacity)
	{
		CurrentMaxAmmo = MaxAmmoCapacity;
		return true;
	}

	return false;
}
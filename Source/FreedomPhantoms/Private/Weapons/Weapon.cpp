#include "Weapons/Weapon.h"
#include "Weapons/WeaponClip.h"
#include "Weapons/Projectile.h"
#include "Weapons/WeaponAttachment.h"
#include "Characters/CombatCharacter.h"
#include "FreedomPhantoms/FreedomPhantoms.h"

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
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "CustomComponents/ObjectPoolComponent.h"
#include "TimerManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundBase.h"
#include "Engine.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/GameInstance.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComp->CanCharacterStepUpOn = ECB_No;

	ObjectPoolComponent = CreateDefaultSubobject<UObjectPoolComponent>(TEXT("ObjectPoolComponent"));
	UseObjectPool = true;

	ShotAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ShotAudioComponent"));
	ShotAudioComponent->SetupAttachment(MeshComp);

	ClipAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ClipAudioComponent"));
	ClipAudioComponent->SetupAttachment(MeshComp);

	ChargingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ChargingAudioComponent"));
	ChargingAudioComponent->SetupAttachment(MeshComp);

	MuzzleLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleLightComponent"));
	MuzzleLightComponent->SetupAttachment(MeshComp);
	MuzzleLightComponent->Intensity = 2000.f;
	MuzzleLightComponent->AttenuationRadius = 500.0f;
	MuzzleLightComponent->SetCastShadows(false);
	MuzzleLightComponent->SetVisibility(false);
	MuzzleLightComponent->SetHiddenInGame(true, true);

	MuzzleSocket = "Muzzle";
	ClipSocket = "Clip";
	HolsterSocket = "holster1";
	EjectorSocket = "Ejector";
	HandguardSocket = "tag_ik_loc_le";
	ReloadClipHandSocket = "clip_hand";
	OpticsSocket = "Optics";
	LaserSocket = "Laser";
	TorchlightSocket = "Torchlight";
	ParentHolderSocket = "Hand";

	MaxAmmoCapacity = 120;
	RateOfFire = 600.0f;
	CooldownReload = 0.0f;

	CrosshairErrorTolerance = .7f;

	BulletSpreadMin = 2.f;
	BulletSpreadMax = 5.f;
	BulletSpreadReduceRate = .1f;
	UseRadialSpread = false;

	ZoomFOV = 90.0f;
	BulletsPerFire = 1;

	UseParentMuzzle = false;
	isReloading = false;
	canShowClip = true;
	hasRecoil = true;

	HasPlayedClipIn = false;
	HasPlayedClipOut = false;

	PlayFireSoundAtLocation = true;

	HasUnlimitedAmmo = false;
	CanAutoReload = false;
	HasNoReload = false;
	HasFiredFirstShot = false;
	ShouldStopFiring = false;

	DrawDebugShotLine = false;
	ShotLineDuration = 5.0f;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	MuzzleLightComponent->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MuzzleSocket);
	ShotAudioComponent->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MuzzleSocket);
	ClipAudioComponent->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ClipSocket);

	SpawnMagazine();
	ConfigSetup();

	GetTimerManager().SetTimer(THandler_DelayedInit, this, &AWeapon::DelayedInit, 1.f, false, 1.f);
}

void AWeapon::DelayedInit()
{
	LoadParentMesh();

	MuzzleLightComponent->AttachToComponent(ParentMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MuzzleSocket);
	ShotAudioComponent->AttachToComponent(ParentMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MuzzleSocket);
	ClipAudioComponent->AttachToComponent(ParentMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ClipSocket);

	GetTimerManager().ClearTimer(THandler_DelayedInit);
}

void AWeapon::SetIsAiming(bool isAiming)
{
	if (!GetMyWorld()) {
		return;
	}

	IsAiming = isAiming;

	if (isAiming)
	{
		ChargeUp();
	}
	else
	{
		ChargeDown();
	}
}

FString AWeapon::GetKeyDisplayName_Implementation()
{
	return FString();
}

FString AWeapon::OnInteractionFound_Implementation(APawn* InPawn, AController* InController)
{
	FString Message = "Pickup ";
	Message.Append(WeaponName.ToString());
	return Message;
}

AActor* AWeapon::OnPickup_Implementation(APawn* InPawn, AController* InController)
{
	if (!InPawn) {
		return nullptr;
	}

	auto CombatCharacter = Cast<ACombatCharacter>(InPawn);

	if (!CombatCharacter) {
		return nullptr;
	}

	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetGenerateOverlapEvents(false);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));

	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation(), 1.f, 1.f, 0.f, FireAttenuation);
	}

	CombatCharacter->PickupWeapon(this);

	return this;
}

bool AWeapon::OnUseInteraction_Implementation(APawn* InPawn, AController* InController)
{
	return false;
}

bool AWeapon::CanInteract_Implementation(APawn* InPawn, AController* InController)
{
	auto OutOwner = GetOwner();
	if (!OutOwner) {
		return true;
	}

	return false;
}

void AWeapon::OnDestroyWeapon(AActor* Actor)
{
	GetMyWorld()->GetTimerManager().ClearAllTimersForObject(this);

	StopFire();

	CurrentChargeAmount = 0.f;

	// Stop playing all audio components
	auto AudioActorComps = GetComponents();
	for (auto& Elem : AudioActorComps)
	{
		auto AudioComp = Cast<UAudioComponent>(Elem);
		if (AudioComp)
		{
			AudioComp->Sound = nullptr;
			AudioComp->Stop();
		}
	}

	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	for (AActor* ChildActor : AttachedActors)
	{
		ChildActor->Destroy();
	}

	Actor->Destroy();
}


void AWeapon::ConfigSetup()
{
	OwningGameInstance = Cast<UGameInstance>(UGameplayStatics::GetGameInstance(GetMyWorld()));

	OnDestroyed.AddDynamic(this, &AWeapon::OnDestroyWeapon);

	ConvertWeaponName();

	HandguardMesh = MeshComp;

	BulletSpreadCurrent = 0.0f;
	BulletSpreadCurrent = BulletSpreadMin;

	SpawnScopeAttachment();

	if (weaponClipObj)
	{
		AmmoPerClip = weaponClipObj->GetAmmoCapacity();
		CurrentAmmo = AmmoPerClip;
	}
	else
	{
		CurrentAmmo = AmmoPerClip;
	}

	TimeBetweenShots = 60 / RateOfFire;

	CurrentMaxAmmo = MaxAmmoCapacity;
	CurrentChargeAmount = 0.0f;


	// If not owner set then it implies this weapon is not possesed by anyone and can be picked up
	if (!GetOwner())
	{
		MeshComp->SetCollisionProfileName(TEXT("Weapon"));
		MeshComp->SetGenerateOverlapEvents(true);
	}
}

void AWeapon::ConvertWeaponName()
{
	if (WeaponName != "None")
	{
		return;
	}

	const UEnum* EnumPtr = FindObject<UEnum>(nullptr, TEXT("/Script/FreedomPhantoms.WeaponType"), true);
	FName WeaponTypeName = EnumPtr->GetNameByValue((int64)weaponType);

	FString Left, Right;
	WeaponTypeName.ToString().Split(TEXT("::"), &Left, &Right);
	WeaponName = *Right;
}

FVector AWeapon::getMuzzleLocation()
{
	if (ParentMesh) {
		return ParentMesh->GetSocketLocation(MuzzleSocket);
	}
	else if (MeshComp && MeshComp->GetSkeletalMeshAsset()) {
		return MeshComp->GetSocketLocation(MuzzleSocket);
	}
	return GetActorLocation();
}

FRotator AWeapon::GetMuzzleRotation()
{
	if (ParentMesh) {
		return ParentMesh->GetSocketRotation(MuzzleSocket);
	}
	else if (MeshComp && MeshComp->GetSkeletalMeshAsset()) {
		return MeshComp->GetSocketRotation(MuzzleSocket);
	}
	return GetActorRotation();
}

void AWeapon::LoadParentMesh()
{
	// Use weapon mesh comp by default
	ParentMesh = MeshComp;

	if (UseParentMuzzle) 
	{
		auto AttachedParent = GetAttachParentActor();

		if (!AttachedParent || AttachedParent->GetComponents().Num() <= 0) {
			return;
		}

		for (auto ChildComponent : AttachedParent->GetComponents())
		{
			auto SceneComp = Cast<UMeshComponent>(ChildComponent);

			if (!SceneComp) {
				continue;
			}

			if (SceneComp->DoesSocketExist(MuzzleSocket)) {
				ParentMesh = SceneComp;
				break;
			}
		}
	}
}

bool AWeapon::IsFacingCrosshair()
{
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
		if (!GetOwner()) {
			return false;
		}

		GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	}

	FVector EyeEnd = UKismetMathLibrary::GetForwardVector(EyeRotation) * TraceLength;

	FVector MuzzleStart = getMuzzleLocation();
	FVector MuzzleEnd = UKismetMathLibrary::GetForwardVector(GetMuzzleRotation()) * TraceLength;

	UKismetMathLibrary::Vector_Normalize(MuzzleEnd);
	UKismetMathLibrary::Vector_Normalize(EyeEnd);
	float Angle = UKismetMathLibrary::Dot_VectorVector(MuzzleEnd, EyeEnd);

	// if dot product is more than x amount, then weapon muzzle is facing in the same direction as the eye location meaning the weapon can fire.
	return Angle > CrosshairErrorTolerance;
}

void AWeapon::Fire()
{
	if (CurrentAmmo <= 0)
	{
		StopFire();

		OnEmptyAmmoClip.Broadcast(this);

		if (!HasNoReload) {
			return;
		}
	}

	if (!CanFireWeapon()) {
		StopFire();
		return;
	}

	// Reset Burst fire count
	if (selectiveFireMode == SelectiveFire::Burst)
	{
		if (BurstAmmountCount >= 3)
		{
			StopFire();
			GetTimerManager().ClearTimer(THandler_BurstFire);
			BurstAmmountCount = 0;
			return;
		}
	}

	isFiring = true;

	if (!HasNoReload) {
		CurrentAmmo -= 1;
	}

	if (weaponClipObj) {
		weaponClipObj->SetCurrentAmmo(CurrentAmmo);
	}

	CreateBullet();

	if (hasRecoil) {

		if (BulletSpreadCurrent < BulletSpreadMax) {
			// Increase current spread based on character velocity
			AActor* MyOwner = GetOwner();
			if (MyOwner) {
				BulletSpreadCurrent += MyOwner->GetVelocity().Size();
			}

			if (IsAiming) {
				BulletSpreadCurrent += .5f;
			}
			else {
				BulletSpreadCurrent += 1.f;
			}
		}
		else {
			BulletSpreadCurrent = BulletSpreadMax;
		}
	}


	if (CanAutoReload && !HasNoReload) {
		if (CurrentAmmo <= 0) {
			isReloading = true;
			ClipOut();
			GetTimerManager().SetTimer(THandler_AutoReloadBegin, this, &AWeapon::AutoReloadBegin, CooldownReload / 2.0f, false);
		}
	}

	if (hasRecoil) {
		GetTimerManager().SetTimer(THandler_BulletSpread, this, &AWeapon::ReduceBulletSpread, BulletSpreadReduceRate, true);
	}

	if (selectiveFireMode == SelectiveFire::Burst) {
		BurstAmmountCount++;
	}

	HasFiredFirstShot = true;

	FWeaponUpdateParameters WeaponUpdateParameters;
	WeaponUpdateParameters.HasFiredShot = true;
	WeaponUpdateParameters.WeaponState = EWeaponState::Firing;
	OnWeaponUpdate.Broadcast(WeaponUpdateParameters);

	if (ShouldStopFiring) {
		StopFire();
	}

	if (CurrentAmmo <= 0) {
		isFiring = false;

		OnEmptyAmmoClip.Broadcast(this);

		if (!HasNoReload)
		{
			return;
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
		FVector TraceEnd = EyeLocation + (ShotDirection * TraceLength);

		if (hasRecoil)
		{
			FVector RandomRadius = BulletSpreadRadial(UKismetMathLibrary::DegTan(BulletSpreadCurrent) * TraceLength);
			TraceEnd += (UKismetMathLibrary::GetRightVector(EyeRotation) * RandomRadius.X) + (UKismetMathLibrary::GetUpVector(EyeRotation) * RandomRadius.Y);
		}


		auto TargetRotation = UKismetMathLibrary::FindLookAtRotation(getMuzzleLocation(), TraceEnd);

		SpawnProjectile(getMuzzleLocation(), TargetRotation);

		if (DrawDebugShotLine)
		{
			DrawDebugLine(GetMyWorld(), EyeLocation, TraceEnd, FColor::Green, true, 1, 0, 2);
		}
	}

	PlayShotEffect(EyeLocation);
}

void AWeapon::SpawnProjectile(FVector Locatiom, FRotator Rotation)
{
	if (BulletClass)
	{
		AProjectile* Bullet = nullptr;

		// Use object pooling if specified
		if (UseObjectPool)
		{
			auto PoolActor = ObjectPoolComponent->ActivatePoolObject(BulletClass, GetOwner(), Locatiom, Rotation, true);

			if (PoolActor)
			{
				Bullet = Cast<AProjectile>(PoolActor);
			}
		}

		// If no bullet object not created
		if (Bullet == nullptr)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Bullet = GetMyWorld()->SpawnActor<AProjectile>(BulletClass, Locatiom, Rotation, SpawnParams);
		}


		if (Bullet)
		{
			Bullet->SetWeaponParent(this);

			if (CanLockActors)
			{
				Bullet->FindHomingTarget(TargetActor);
			}

			if (!Bullet->OnProjectileImpact.IsBound())
			{
				Bullet->OnProjectileImpact.AddDynamic(this, &AWeapon::OnProjectileImpacted);
			}
		}
	}
}

void AWeapon::PlayShotEffect(FVector EyeLocation)
{
	MuzzleLightComponent->SetVisibility(true, true);
	MuzzleLightComponent->SetHiddenInGame(false, true);

	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetMyWorld(), MuzzleEffect, getMuzzleLocation(), GetMuzzleRotation());
	}

	if (MuzzleFlashNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetMyWorld(), MuzzleFlashNiagara, getMuzzleLocation(), GetMuzzleRotation());
	}

	LastFireTime = GetMyWorld()->TimeSeconds;

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

	// to get a blinking muzzle flash, delay rate needs to be very low
	GetTimerManager().SetTimer(THandler_MuzzleLight, this, &AWeapon::DisableMuzzleLight, .01f, false);
}

void AWeapon::OnProjectileImpacted(FProjectileImpactParameters ProjectileImpactParameters)
{
	if (ProjectileImpactParameters.KillCount > 0)
	{
		OnKillConfirmed.Broadcast(ProjectileImpactParameters);
	}

	// As the projectile owner can change, we do not want this event being fired again if the projectile was shot by another character,
	// but first check if the projectile exists, since some weapons may not be using object pooling to respawn the projectile
	if (ProjectileImpactParameters.ProjectileActor)
	{
		ProjectileImpactParameters.ProjectileActor->OnProjectileImpact.Clear();
	}
}

// Bullet spread random point
FVector AWeapon::BulletSpreadRadial(float Radius)
{
	FVector Target;
	float Angle = UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f);
	float DistanceFromCenter = UKismetMathLibrary::RandomFloatInRange(0.0f, Radius);

	Target.X = DistanceFromCenter * UKismetMathLibrary::DegCos(Angle);
	Target.Y = DistanceFromCenter * UKismetMathLibrary::DegSin(Angle);

	return Target;
}

FRotator AWeapon::GetSprayAngle(FVector MuzzleDirection, float MaxAngle)
{
	return UKismetMathLibrary::MakeRotFromX(UKismetMathLibrary::RandomUnitVectorInConeInDegrees(MuzzleDirection, MaxAngle));
}

FVector AWeapon::BulletSpread(FVector Spread)
{
	float Range = UKismetMathLibrary::MapRangeClamped(BulletSpreadCurrent, .0f, 1.f, 10.f, 20.f);
	float SpreadMin = BulletSpreadMin * Range;
	float SpreadMax = BulletSpreadMax * Range;
	float RandomRange = UKismetMathLibrary::RandomFloatInRange(SpreadMin, SpreadMax);

	FVector TargetSpread;
	TargetSpread.X = Spread.X + RandomRange;
	TargetSpread.Y = Spread.Y + RandomRange;
	TargetSpread.Z = Spread.Z + RandomRange;

	return TargetSpread;
}

void AWeapon::ReduceBulletSpread()
{
	BulletSpreadCurrent = FMath::Clamp(BulletSpreadCurrent - 1.f, BulletSpreadMin, BulletSpreadMax);

	if (BulletSpreadCurrent <= BulletSpreadMin)
	{
		BulletSpreadCurrent = BulletSpreadMin;
		GetTimerManager().ClearTimer(THandler_BulletSpread);
	}
}

void AWeapon::BeginShellEffect()
{
	if (ShellEjectEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetMyWorld(), ShellEjectEffect, MeshComp->GetSocketLocation(EjectorSocket));
	}
}

void AWeapon::StartFire()
{
	ShouldStopFiring = false;

	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetMyWorld()->TimeSeconds, 0.0f);

	switch (selectiveFireMode)
	{
	case SelectiveFire::Automatic:
		if (!THandler_TimeBetweenShots.IsValid()) {
			GetTimerManager().SetTimer(THandler_TimeBetweenShots, this, &AWeapon::Fire, TimeBetweenShots, true, FirstDelay);
		}
		break;
	case SelectiveFire::SemiAutomatic:
		GetTimerManager().SetTimer(THandler_TimeBetweenShots, this, &AWeapon::Fire, TimeBetweenShots, false, FirstDelay);
		break;
	case SelectiveFire::Burst:
		if (!THandler_BurstFire.IsValid()) {
			GetTimerManager().SetTimer(THandler_BurstFire, this, &AWeapon::Fire, TimeBetweenShots, true, FirstDelay);
		}
		break;
	default:
		Fire();
		break;
	}

	ChargeUp();
}

void AWeapon::StopFire()
{
	if (!GetMyWorld() || !HasFiredFirstShot) {
		ShouldStopFiring = true;
		return;
	}

	GetTimerManager().ClearTimer(THandler_TimeBetweenShots);

	// reset flags
	isFiring = false;
	HasFiredFirstShot = false;
	ShouldStopFiring = false;

	ChargeDown();

	if (!THandler_BulletSpread.IsValid()) {
		GetTimerManager().SetTimer(THandler_BulletSpread, this, &AWeapon::ReduceBulletSpread, BulletSpreadReduceRate, true);
	}

	FWeaponUpdateParameters WeaponUpdateParameters;
	WeaponUpdateParameters.HasFiredShot = true;
	WeaponUpdateParameters.WeaponState = EWeaponState::Default;
	OnWeaponUpdate.Broadcast(WeaponUpdateParameters);
}

void AWeapon::ReadyToUse()
{
	HasFiredFirstShot = false;

	StopFire();
}

bool AWeapon::CanFireWeapon()
{
	if (!GetOwner()) {
		return false;
	}

	if (THandler_DelayedInit.IsValid() || CurrentAmmo <= 0) {
		return false;
	}

	if (isReloading) {
		return false;
	}

	// check if facing crosshair if tolerance is set to more than zero.
	if (CrosshairErrorTolerance > 0.f && !IsFacingCrosshair()) {
		return false;
	}

	return true;
}

void AWeapon::ChargeUp()
{
	if (IsChargingUp) {
		return;
	}

	if (THandler_ChargeDown.IsValid()) {
		GetTimerManager().ClearTimer(THandler_ChargeDown);
	}


	IsChargingUp = true;

	if (!THandler_ChargeUp.IsValid()) {
		GetTimerManager().SetTimer(THandler_ChargeUp, this, &AWeapon::IncreaseCharge, .1f, true);
	}

}

void AWeapon::ChargeDown()
{
	if (isFiring) {
		return;
	}

	if (IsAiming) {
		return;
	}

	if (THandler_ChargeUp.IsValid()) {
		GetTimerManager().ClearTimer(THandler_ChargeUp);
	}

	IsChargingUp = false;

	if (!THandler_ChargeDown.IsValid()) {
		GetTimerManager().SetTimer(THandler_ChargeDown, this, &AWeapon::DecreaseCharge, .1f, true);
	}
}

void AWeapon::IncreaseCharge()
{
	CurrentChargeAmount = FMath::Clamp(CurrentChargeAmount + .1f, 0.f, ChargeUpTime);

	// Start from the last item, check against if the current charge amount is greater than current iteration start amount
	for (int i = ChargeUpSounds.Num() - 1; i > 0; i--)
	{
		auto Sound = ChargeUpSounds[i];

		// if already played & not set to loop then ignore this item
		if (Sound.AudioComponent && !Sound.IsLoop) {
			continue;
		}

		// if current charge amount has reached or greater than current item's start amount
		if (CurrentChargeAmount >= Sound.StartAmount) {

			bool CanPlay = true;

			// if meant to be looping but not playing, play again
			if (Sound.IsLoop && Sound.AudioComponent && Sound.AudioComponent->IsPlaying()) {
				CanPlay = false;
			}

			if (CanPlay) {
				Sound.AudioComponent = UGameplayStatics::SpawnSoundAtLocation(GetMyWorld(), Sound.Audio, GetActorLocation(), FRotator::ZeroRotator, .5f, 1.f, 0.f, ChargingAudioComponent->AttenuationSettings);
			}

			// update the array struct 
			ChargeUpSounds[i] = Sound;
			break;
		}
	}
}

void AWeapon::DecreaseCharge()
{
	if (!GetMyWorld()) {
		return;
	}

	for (int i = 0; i < ChargeUpSounds.Num(); i++)
	{
		auto Sound = ChargeUpSounds[i];
		if (Sound.AudioComponent) {
			Sound.AudioComponent->Stop();
			Sound.AudioComponent = nullptr;
			ChargeUpSounds[i] = Sound;
		}
	}


	for (int i = 0; i < ChargeDownSounds.Num(); i++)
	{
		auto Sound = ChargeDownSounds[i];
		if (CurrentChargeAmount >= Sound.StartAmount && !Sound.AudioComponent) {
			Sound.AudioComponent = UGameplayStatics::SpawnSoundAtLocation(GetMyWorld(), Sound.Audio, GetActorLocation(), FRotator::ZeroRotator, .5f, 1.f, 0.f, ChargingAudioComponent->AttenuationSettings);
			ChargeDownSounds[i] = Sound;
			break;
		}
	}

	if (CurrentChargeAmount <= 0.0f)
	{
		for (int i = 0; i < ChargeDownSounds.Num(); i++)
		{
			auto Sound = ChargeDownSounds[i];
			if (Sound.AudioComponent) {
				Sound.AudioComponent = nullptr;
				ChargeDownSounds[i] = Sound;
			}
		}

		GetTimerManager().ClearTimer(THandler_ChargeDown);
	}

	CurrentChargeAmount = FMath::Clamp(CurrentChargeAmount - .1f, 0.f, ChargeUpTime);
}

void AWeapon::DisableMuzzleLight()
{
	MuzzleLightComponent->SetVisibility(false, true);
	MuzzleLightComponent->SetHiddenInGame(true, true);

	if (GetMyWorld() && THandler_MuzzleLight.IsValid()) {
		GetTimerManager().ClearTimer(THandler_MuzzleLight);
	}
}

void AWeapon::OnReload()
{
	if (!GetMyWorld()) {
		return;
	}

	// Do we have ammo in the ammopool?
	if (CurrentMaxAmmo <= 0 || CurrentAmmo >= AmmoPerClip && !HasUnlimitedAmmo) {
		return;
	}

	GetTimerManager().ClearTimer(THandler_TimeBetweenShots);

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

	GetTimerManager().ClearTimer(THandler_TimeBetweenShots);

	StopFire();
	isReloading = true;

	if (weaponClipObj) {
		weaponClipObj->getClipMesh()->SetVisibility(false);
	}

	FWeaponUpdateParameters WeaponUpdateParameters;
	WeaponUpdateParameters.WeaponState = EWeaponState::Reloading;
	OnWeaponUpdate.Broadcast(WeaponUpdateParameters);
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


	FWeaponUpdateParameters WeaponUpdateParameters;
	WeaponUpdateParameters.WeaponState = EWeaponState::ReloadEnded;
	OnWeaponUpdate.Broadcast(WeaponUpdateParameters);
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

void AWeapon::EmptyClipEvent()
{
	if (CurrentAmmo <= 0) {
		isFiring = false;

		OnEmptyAmmoClip.Broadcast(this);

		if (!HasNoReload) {
			return;
		}
	}
}

void AWeapon::SpawnMagazine()
{
	if (!weaponClip) {
		return;
	}

	UWorld* world = GetMyWorld();

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

void AWeapon::SpawnScopeAttachment()
{
	if (ScopeAttachmentClasses.Num() <= 0) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	auto RandomIndex = rand() % ScopeAttachmentClasses.Num();

	ScopeAttachment = GetMyWorld()->SpawnActor<AWeaponAttachment>(ScopeAttachmentClasses[RandomIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (ScopeAttachment)
	{
		ScopeAttachment->GetRootComponent()->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ScopeAttachment->GetParentSocket());
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

void AWeapon::HolsterWeapon(USkeletalMeshComponent* Parent)
{
	MeshComp->AttachToComponent(Parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HolsterSocket);
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

	FWeaponUpdateParameters WeaponUpdateParameters;
	WeaponUpdateParameters.WeaponState = EWeaponState::Reloading;
	OnWeaponUpdate.Broadcast(WeaponUpdateParameters);

	if (GetMyWorld())
	{
		GetTimerManager().SetTimer(THandler_AutoReloadEnd, this, &AWeapon::AutoReloadEnd, CooldownReload / 2.0f, false);
		GetTimerManager().ClearTimer(THandler_AutoReloadBegin);
	}
}

void AWeapon::AutoReloadEnd()
{
	OnReload();
	EndReload();
	isReloading = false;

	GetTimerManager().ClearTimer(THandler_AutoReloadEnd);
}


bool AWeapon::ReplenishAmmo(int Amount)
{
	// If amount is less than zero specified, then replenish to full capactiy.
	if (Amount < 0)
	{
		if (CurrentMaxAmmo < MaxAmmoCapacity)
		{
			CurrentMaxAmmo = MaxAmmoCapacity;
			return true;
		}
	}
	else
	{
		if (CurrentMaxAmmo < MaxAmmoCapacity)
		{
			CurrentMaxAmmo = FMath::Clamp(CurrentMaxAmmo + Amount, 0, MaxAmmoCapacity);
			return true;
		}
	}

	return false;
}

void AWeapon::ToggleVisibility(bool Enabled)
{
	if (Enabled) {
		MeshComp->SetHiddenInGame(false, true);
	}
	else {
		MeshComp->SetHiddenInGame(true, true);
		SetActorEnableCollision(false);
	}
}

void AWeapon::SetWeaponProfile(FName InCollisionProfileName)
{
	//MeshComp->SetCollisionProfileName(InCollisionProfileName);
}

void AWeapon::DropWeapon(bool RemoveOwner, bool SimulatePhysics)
{
	if (RemoveOwner)
	{
		SetOwner(nullptr);
	}

	HasUnlimitedAmmo = false;
	EndReload();
	IsAiming = false;

	// character can be reloading when the weapon is dropped,
	// the clip will need to be set back in the weapon socket
	SetMagazineSocket();

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	MeshComp->SetCollisionProfileName(TEXT("Weapon"));
	MeshComp->SetGenerateOverlapEvents(true);
	MeshComp->SetSimulatePhysics(SimulatePhysics);

}

UWorld* AWeapon::GetMyWorld() const
{
	if (GetWorld())
	{
		return GetWorld();
	}

	return OwningGameInstance->GetWorld();
}


FTimerManager& AWeapon::GetTimerManager() const
{
	if (GetWorld())
	{
		return GetWorld()->GetTimerManager();
	}

	return OwningGameInstance->GetTimerManager();
}
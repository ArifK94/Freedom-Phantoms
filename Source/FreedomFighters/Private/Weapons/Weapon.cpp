#include "Weapons/Weapon.h"
#include "Weapons/WeaponClip.h"
#include "Weapons/WeaponBullet.h"
#include "Weapons/WeaponAttachment.h"
#include "Characters/CombatCharacter.h"
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
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
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

	ChargeSoundParamName = "ChargeAmount";

	DrawShotLine = false;
	ShotLineDuration = 5.0f;
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
	CurrentChargeUpTime = 0.0f;


	// If not owner set then it implies this weapon is not possesed by anyone and can be picked up
	if (!GetOwner())
	{
		MeshComp->SetCollisionProfileName(TEXT("Weapon"));
		MeshComp->SetGenerateOverlapEvents(true);
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

	ScopeAttachment = GetWorld()->SpawnActor<AWeaponAttachment>(ScopeAttachmentClasses[RandomIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (ScopeAttachment)
	{
		ScopeAttachment->GetRootComponent()->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ScopeAttachment->GetParentSocket());
	}

}

void AWeapon::ConvertWeaponName()
{
	if (WeaponName != "None")
	{
		return;
	}

	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("WeaponType"), true);
	FName WeaponTypeName = EnumPtr->GetNameByValue((int64)weaponType);

	FString Left, Right;
	WeaponTypeName.ToString().Split(TEXT("::"), &Left, &Right);
	WeaponName = *Right;
}

FVector AWeapon::getMuzzleLocation()
{
	if (UseParentMuzzle) {
		auto AttachedParent = GetAttachParentActor();

		if (AttachedParent) {
			return AttachedParent->GetRootComponent()->GetSocketLocation(MuzzleSocket);
		}
	}

	return MeshComp->GetSocketLocation(MuzzleSocket);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	MuzzleLightComponent->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MuzzleSocket);
	ShotAudioComponent->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MuzzleSocket);
	ClipAudioComponent->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ClipSocket);

	GetWorldTimerManager().SetTimer(THandler_DelayedInit, this, &AWeapon::DelayedInit, 1.f, true);

	SpawnMagazine();
	ConfigSetup();
}

void AWeapon::DelayedInit()
{
	auto AttachedParent = GetAttachParentActor();
	if (UseParentMuzzle && AttachedParent)
	{
		MuzzleLightComponent->AttachToComponent(AttachedParent->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, MuzzleSocket);
		ShotAudioComponent->AttachToComponent(AttachedParent->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, MuzzleSocket);
		ClipAudioComponent->AttachToComponent(AttachedParent->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ClipSocket);
	}


	GetWorldTimerManager().ClearTimer(THandler_DelayedInit);
}

void AWeapon::Fire()
{
	if (isReloading) {
		return;
	}

	if (CurrentAmmo <= 0) {
		isFiring = false;

		OnEmptyAmmoClip.Broadcast(this);

		if (!HasNoReload)
		{
			return;
		}
	}

	// Reset Burst fire count
	if (selectiveFireMode == SelectiveFire::Burst)
	{
		if (BurstAmmountCount >= 3)
		{
			StopFire();
			GetWorldTimerManager().ClearTimer(THandler_BurstFire);
			BurstAmmountCount = 0;
			return;
		}
	}

	isFiring = true;

	if (!HasNoReload) {
		CurrentAmmo -= 1;
	}

	if (weaponClipObj)
	{
		weaponClipObj->SetCurrentAmmo(CurrentAmmo);
	}

	CreateBullet();

	if (hasRecoil) {

		if (BulletSpreadCurrent < BulletSpreadMax)
		{
			// Increase current spread based on character velocity
			AActor* MyOwner = GetOwner();
			if (MyOwner) {
				BulletSpreadCurrent += MyOwner->GetVelocity().Size();
			}

			if (IsAiming)
			{
				BulletSpreadCurrent += .5f;
			}
			else
			{
				BulletSpreadCurrent += 1.f;
			}
		}
		else
		{
			BulletSpreadCurrent = BulletSpreadMax;
		}
	}


	if (CanAutoReload && !HasNoReload)
	{
		if (CurrentAmmo <= 0)
		{
			isReloading = true;
			ClipOut();
			GetWorldTimerManager().SetTimer(THandler_AutoReloadBegin, this, &AWeapon::AutoReloadBegin, CooldownReload / 2.0f, false);
		}
	}

	if (hasRecoil) {
		GetWorldTimerManager().SetTimer(THandler_BulletSpread, this, &AWeapon::ReduceBulletSpread, BulletSpreadReduceRate, true);
	}

	if (selectiveFireMode == SelectiveFire::Burst)
	{
		BurstAmmountCount++;
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
			if (UseRadialSpread)
			{
				FVector RandomRadius = BulletSpreadRadial(UKismetMathLibrary::DegTan(BulletSpreadCurrent) * TraceLength);
				TraceEnd += (UKismetMathLibrary::GetRightVector(EyeRotation) * RandomRadius.X) + (UKismetMathLibrary::GetUpVector(EyeRotation) * RandomRadius.Y);
			}
			else
			{
				TraceEnd = BulletSpread(TraceEnd);
			}
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
			AWeaponBullet* Bullet = nullptr;

			// Use object pooling if specified
			if (UseObjectPool)
			{
				auto PoolActor = ObjectPoolComponent->ActivatePoolObject(BulletClass, MyOwner, getMuzzleLocation(), UKismetMathLibrary::FindLookAtRotation(getMuzzleLocation(), TracerEndPoint), true);

				if (PoolActor)
				{
					Bullet = Cast<AWeaponBullet>(PoolActor);
				}
			}

			// If no bullet object not created
			if (Bullet == nullptr)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = MyOwner;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				Bullet = GetWorld()->SpawnActor<AWeaponBullet>(BulletClass, getMuzzleLocation(), UKismetMathLibrary::FindLookAtRotation(getMuzzleLocation(), TracerEndPoint), SpawnParams);
			}


			if (Bullet)
			{
				Bullet->SetWeaponParent(this);
			}
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
	MuzzleLightComponent->SetVisibility(true);

	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleEffect, getMuzzleLocation(), MeshComp->GetSocketRotation(MuzzleSocket));
	}

	if (MuzzleFlashNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashNiagara, getMuzzleLocation(), MeshComp->GetSocketRotation(MuzzleSocket));
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

	// to get a blinking muzzle flash, delay rate needs to be very low
	GetWorldTimerManager().SetTimer(THandler_MuzzleLight, this, &AWeapon::DisableMuzzleLight, .01f, false);
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
		GetWorldTimerManager().ClearTimer(THandler_BulletSpread);
	}
}

void AWeapon::BeginShellEffect()
{
	if (ShellEjectEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShellEjectEffect, MeshComp->GetSocketLocation(EjectorSocket));
	}
}

void AWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	switch (selectiveFireMode)
	{
	case SelectiveFire::Automatic:
		if (!THandler_TimeBetweenShots.IsValid()) {
			GetWorldTimerManager().SetTimer(THandler_TimeBetweenShots, this, &AWeapon::Fire, TimeBetweenShots, true, FirstDelay);
		}
		break;
	case SelectiveFire::SemiAutomatic:
		if (!THandler_TimeBetweenShots.IsValid()) {
			GetWorldTimerManager().SetTimer(THandler_TimeBetweenShots, this, &AWeapon::Fire, TimeBetweenShots, false, FirstDelay);
		}
		break;
	case SelectiveFire::Burst:
		if (!THandler_BurstFire.IsValid()) {
			GetWorldTimerManager().SetTimer(THandler_BurstFire, this, &AWeapon::Fire, TimeBetweenShots, true, FirstDelay);
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
	GetWorldTimerManager().ClearTimer(THandler_TimeBetweenShots);

	isFiring = false;

	CurrentVerticleRecoil = 0.0f;

	ChargeDown();

	GetWorldTimerManager().SetTimer(THandler_BulletSpread, this, &AWeapon::ReduceBulletSpread, BulletSpreadReduceRate, true);
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

		if (ChargeDownSound != nullptr)
		{
			ChargingAudioComponent->Stop();
		}

		GetWorldTimerManager().ClearTimer(THandler_ChargeDown);
	}
}

void AWeapon::DisableMuzzleLight()
{
	MuzzleLightComponent->SetVisibility(false);
	GetWorldTimerManager().ClearTimer(THandler_MuzzleLight);
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




bool AWeapon::ReplenishAmmo(int Amount)
{
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
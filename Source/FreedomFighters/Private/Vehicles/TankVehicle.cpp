// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/TankVehicle.h"
#include "Vehicles/LandVehicle.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"
#include "CustomComponents/ShooterComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/HealthComponent.h"

#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"


FVehicleWeapon ATankVehicle::GetCurrentVehicleWeapon()
{
	if (CurrentWeapon == MainWeapon)
	{
		return VehicleWeaponMain;
	}
	else if (CurrentWeapon == SecondaryWeapons[CurrentWeaponIndex])
	{
		return VehicleWeaponTurrets[CurrentWeaponIndex];
	}
	return FVehicleWeapon();
}

void ATankVehicle::SetRotationInput(FRotator InRotation)
{
	//auto CurrentRotation = RotationInput;
	//auto TotalPitch = RotationInput.Pitch + InRotation.Pitch;
	//auto TotalYaw = RotationInput.Yaw + InRotation.Yaw;

	//ChangePitchValue(TotalYaw);

	//if (TotalPitch >= PitchMax) {
	//	TotalPitch = PitchMax;
	//}
	//else if (TotalPitch <= PitchMin) {
	//	TotalPitch = PitchMin;
	//}
	//else {
	//	TotalPitch = InRotation.Pitch;
	//}

	//if (TotalYaw >= YawMax) {
	//	TotalYaw = YawMax;
	//}
	//else if (TotalYaw <= YawMin) {
	//	TotalYaw = YawMin;
	//}
	//else {
	//	TotalYaw = InRotation.Yaw;
	//}

	//RotationInput.Pitch = TotalPitch;
	//RotationInput.Yaw = TotalYaw;

	RotationInput = InRotation;
}

void ATankVehicle::SetCurrentWeapon(AMountedGun* InMountedGun, FVehicleWeapon InVehicleWeapon)
{
	CurrentWeapon = InMountedGun;
	ShooterComponent->SetWeapon(CurrentWeapon);
}

ATankVehicle::ATankVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	TurretAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("TurretAudio"));
	TurretAudio->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepRelativeTransform);

	TeamFactionComponent = CreateDefaultSubobject<UTeamFactionComponent>(TEXT("TeamFactionComponent"));

	TargetFinderComponent = CreateDefaultSubobject<UTargetFinderComponent>(TEXT("TargetFinderComponent"));
	TargetFinderComponent->SetFindTargetPerFrame(true);
	TargetFinderComponent->AddClassFilter(ATankVehicle::StaticClass());

	ShooterComponent = CreateDefaultSubobject<UShooterComponent>(TEXT("ShooterComponent"));

	CurrentWeaponIndex = 0;

	TurretRotationFactor = 1.5f;
	TurretRotationErrorTolerance = 5.f;
}

void ATankVehicle::BeginPlay()
{
	Super::BeginPlay();

	TargetFinderComponent->OnTargetSearch.AddDynamic(this, &ATankVehicle::OnTargetSearchUpdate);
	DefaultPitchMin = PitchMin;
	DefaultPitchMax = PitchMax;
	DefaultYawMin = YawMin;
	DefaultYawMax = YawMax;

	MainWeapon = SpawnVehicleWeapon(VehicleWeaponMain);

	for (int i = 0; i < VehicleWeaponTurrets.Num(); i++)
	{
		auto Weapon = SpawnVehicleWeapon(VehicleWeaponTurrets[i]);

		if (Weapon) {
			SecondaryWeapons.Add(Weapon);
		}
	}

	if (MainWeapon)
	{
		SetCurrentWeapon(MainWeapon, VehicleWeaponMain);
	}
	else if (SecondaryWeapons.Num() > 0)
	{
		SetCurrentWeapon(SecondaryWeapons[CurrentWeaponIndex], VehicleWeaponTurrets[CurrentWeaponIndex]);
	}
}

void ATankVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_DeltaTime = DeltaTime;

	Shoot();
}

void ATankVehicle::OnHealthUpdate(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo)
{
	Super::OnHealthUpdate(OwningHealthComp, Health, HealthDelta, DamageType, InstigatedBy, DamageCauser, WeaponCauser, Bullet, HitInfo);

	if (!HealthComp->IsAlive())
	{
		//PrimaryActorTick.bCanEverTick = false;
		ShooterComponent->EndFire();
	}
}

void ATankVehicle::OnTargetSearchUpdate(AActor* Actor)
{
	TargetActor = Actor;

	if (!Actor) {
		ShooterComponent->EndFire();
		GetWorldTimerManager().ClearTimer(THandler_RandomChangeWeapon);
		return;
	}

	// Use main weapon on vehicles
	if (Actor->IsA(ALandVehicle::StaticClass()))
	{
		if (CurrentWeapon != MainWeapon)
		{
			SetCurrentWeapon(MainWeapon, VehicleWeaponMain);
		}
	}
	else // else target is most likely infantry
	{
		// add more character to tank by radnomly changing weapons at random times
		if (!THandler_RandomChangeWeapon.IsValid())
		{
			// Change to secondary weapons at first then randomly change the weapons a few x seconds later
			ChangeSecondaryWeapon();

			GetWorldTimerManager().SetTimer(THandler_RandomChangeWeapon, this, &ATankVehicle::RandomChangeWeapon, FMath::RandRange(5.f, 10.f), true);
		}


	}
}

AMountedGun* ATankVehicle::SpawnVehicleWeapon(FVehicleWeapon VehicleWeapon)
{
	if (!VehicleWeapon.WeaponClass) {
		return nullptr;
	}

	auto MyOwner = GetOwner() ? GetOwner() : this;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = MyOwner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	auto Weapon = GetWorld()->SpawnActor<AMountedGun>(VehicleWeapon.WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (!Weapon) {
		return nullptr;
	}

	Weapon->SetOwner(MyOwner);
	Weapon->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleWeapon.WeaponSocketName);
	Weapon->SetWeaponProfile(TEXT("NoCollision"));
	return Weapon;
}

void ATankVehicle::ChangeSecondaryWeapon()
{
	if (SecondaryWeapons.Num() <= 0) {
		return;
	}

	// increment the index if current index is less than the array of weapons
	// otherwise go back to the first index
	if (CurrentWeaponIndex < SecondaryWeapons.Num() - 1)
	{
		CurrentWeaponIndex++;
	}
	else
	{
		CurrentWeaponIndex = 0;
	}

	SetCurrentWeapon(SecondaryWeapons[CurrentWeaponIndex], VehicleWeaponTurrets[CurrentWeaponIndex]);

}

void ATankVehicle::RandomChangeWeapon()
{
	auto RandomBool = UKismetMathLibrary::RandomBool();

	if (RandomBool)
	{
		SetCurrentWeapon(MainWeapon, VehicleWeaponMain);
	}
	else
	{
		ChangeSecondaryWeapon();
	}
}

FRotator ATankVehicle::FaceTarget(AActor* Actor, FRotator& TargetRotation)
{
	if (!Actor) {
		TargetRotation = FRotator::ZeroRotator;
		return  UKismetMathLibrary::RLerp(RotationInput, TargetRotation, m_DeltaTime * TurretRotationFactor, false);
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	GetActorEyesViewPoint(EyeLocation, EyeRotation); // Grab the camera view points, this is a fail safe if vehicle weapons do not exist for some reason

	auto VehicleWeapon = GetCurrentVehicleWeapon();

	// eye location should be retrieved from weapon socket location as the follow camera shouldn't change as it can be used by another mechanic such as orbiting around the vehicle
	// check if weapon class was provided as we cannot access the weapon object pointer when it has UPROPERTY attribute
	if (VehicleWeapon.WeaponClass)
	{
		EyeLocation = MeshComp->GetSocketLocation(VehicleWeapon.WeaponSocketName);
	}

	auto TargetLocation = Actor->GetActorLocation() - EyeLocation;
	auto RootBone = MeshComp->GetBoneName(0);
	auto TargetDirectionInvert = UKismetMathLibrary::InverseTransformDirection(MeshComp->GetSocketTransform(RootBone), TargetLocation);
	TargetRotation = UKismetMathLibrary::MakeRotFromX(TargetDirectionInvert);

	return UKismetMathLibrary::RInterpTo(RotationInput, TargetRotation, GetWorld()->DeltaTimeSeconds, TurretRotationFactor);

}

void ATankVehicle::Shoot()
{
	if (!HealthComp->IsAlive()) {
		return;
	}

	auto TargetRotation = FRotator::ZeroRotator;
	auto NewRotationInput = FaceTarget(TargetActor, TargetRotation);
	SetRotationInput(NewRotationInput);

	auto NearlyEqualPitch = UKismetMathLibrary::NearlyEqual_FloatFloat(RotationInput.Pitch, TargetRotation.Pitch, TurretRotationErrorTolerance);
	auto NearlyEqualYaw = UKismetMathLibrary::NearlyEqual_FloatFloat(RotationInput.Yaw, TargetRotation.Yaw, TurretRotationErrorTolerance);
	auto NearlyEqualRoll = UKismetMathLibrary::NearlyEqual_FloatFloat(RotationInput.Roll, TargetRotation.Roll, TurretRotationErrorTolerance);
	auto NearlyEqual = NearlyEqualPitch && NearlyEqualYaw && NearlyEqualRoll;

	// Rotation nearly turned to target rotation, can be considered as reaching target rotation
	if (NearlyEqual)
	{
		if (TargetActor)
		{
			ShooterComponent->BeginFire();
		}
		else
		{
			ShooterComponent->EndFire();
		}

		TurretAudio->Sound = TurretTurnStopSound;
		TurretAudio->Play();

	}
	else // is turning to target rotation
	{
		TurretAudio->Sound = TurretTurnSound;
		if (!TurretAudio->IsPlaying()) {
			TurretAudio->Play();
		}

		ShooterComponent->EndFire();
	}
}

void ATankVehicle::ChangePitchValue(float InYawValue)
{
	if (ClampChangePitchValues.Num() <= 0) {
		return;
	}

	auto NewPitchMin = DefaultPitchMin;
	auto NewPitchMax = DefaultPitchMax;

	for (int i = 0; i < ClampChangePitchValues.Num(); i++)
	{
		auto NewClampVal = ClampChangePitchValues[i];

		if (InYawValue >= NewClampVal.YawValueMin && InYawValue <= NewClampVal.YawValueMax)
		{
			if (!NewClampVal.UseMinDefault) {
				NewPitchMin = NewClampVal.NewPitchMin;
			}

			if (!NewClampVal.UseMaxDefault) {
				NewPitchMax = NewClampVal.NewPitchMax;
			}
			break;
		}
	}

	PitchMin = NewPitchMin;
	PitchMax = NewPitchMax;
}
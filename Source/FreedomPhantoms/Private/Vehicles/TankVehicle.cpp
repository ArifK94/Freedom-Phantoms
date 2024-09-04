// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/TankVehicle.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"
#include "CustomComponents/ShooterComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/VehiclePathFollowerComponent.h"

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



void ATankVehicle::SetCurrentWeapon(AMountedGun* InMountedGun, FVehicleWeapon InVehicleWeapon)
{
	CurrentWeapon = InMountedGun;

	TArray<AWeapon*> Weapons;
	Weapons.Add(CurrentWeapon);
	ShooterComponent->SetWeapons(Weapons);
}

ATankVehicle::ATankVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	TurretAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("TurretAudio"));
	TurretAudio->AttachToComponent(MeshComponent, FAttachmentTransformRules::KeepRelativeTransform);

	TeamFactionComponent = CreateDefaultSubobject<UTeamFactionComponent>(TEXT("TeamFactionComponent"));

	TargetFinderComponent = CreateDefaultSubobject<UTargetFinderComponent>(TEXT("TargetFinderComponent"));
	TargetFinderComponent->AddClassFilter(ATankVehicle::StaticClass());
	TargetFinderComponent->FindTargetPerFrame = true;

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

bool ATankVehicle::ShouldStopVehicle()
{
	if (TargetActor && StopOnTargetFound)
	{
		return true;
	}
	else if (!TargetActor && StopOnTargetFound && VehiclePathFollowerComponent)
	{
		VehiclePathFollowerComponent->ResumePath();
	}

	return Super::ShouldStopVehicle();
}

void ATankVehicle::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	Super::OnHealthUpdate(InHealthParameters);

	if (!HealthComponent->IsAlive())
	{
		TargetFinderComponent->SetFindTargetPerFrame(false);
		ShooterComponent->StopFiringWeapons();

		// Destroy all weapons
		if (MainWeapon) {
			MainWeapon->Destroy();
		}

		for (int i = 0; i < SecondaryWeapons.Num(); i++)
		{
			auto Weapon = SecondaryWeapons[i];

			if (Weapon) {
				Weapon->Destroy();
			}
		}
	}
}

void ATankVehicle::OnTargetSearchUpdate(FTargetSearchParameters TargetSearchParameters)
{
	TargetSearchParams = TargetSearchParameters;

	TargetActor = TargetSearchParameters.TargetActor;

	if (!TargetActor) {

		if (VehiclePathFollowerComponent)
		{
			VehiclePathFollowerComponent->ResumeNormalSpeed();
		}

		ShooterComponent->StopFiringWeapons();
		GetWorldTimerManager().ClearTimer(THandler_RandomChangeWeapon);
		return;
	}

	// slowdown when target actor is found.
	if (SlowdownOnTargetFound && VehiclePathFollowerComponent) {
		VehiclePathFollowerComponent->Slowdown();
	}

	// Use main weapon on vehicles
	if (TargetActor->IsA(AVehicleBase::StaticClass()))
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
	Weapon->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleWeapon.WeaponSocketName);
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
		EyeLocation = MeshComponent->GetSocketLocation(VehicleWeapon.WeaponSocketName);
	}

	auto TargetLocation = TargetSearchParams.TargetLocation - EyeLocation;
	auto RootBone = MeshComponent->GetBoneName(0);
	auto TargetDirectionInvert = UKismetMathLibrary::InverseTransformDirection(MeshComponent->GetSocketTransform(RootBone), TargetLocation);
	TargetRotation = UKismetMathLibrary::MakeRotFromX(TargetDirectionInvert);

	return UKismetMathLibrary::RInterpTo(RotationInput, TargetRotation, GetWorld()->DeltaTimeSeconds, TurretRotationFactor);

}

void ATankVehicle::Shoot()
{
	if (!HealthComponent->IsAlive()) {
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
			ShooterComponent->StopFiringWeapons();
		}

		TurretAudio->Stop();
		TurretAudio->Sound = TurretTurnStopSound;
		TurretAudio->Play();

	}
	else // is turning to target rotation
	{
		TurretAudio->Sound = TurretTurnSound;
		if (!TurretAudio->IsPlaying()) {
			TurretAudio->Play();
		}

		ShooterComponent->StopFiringWeapons();
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
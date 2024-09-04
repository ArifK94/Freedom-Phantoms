// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/Aircraft.h"
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


AAircraft::AAircraft()
{
	PrimaryActorTick.bCanEverTick = true;

	TurretAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("TurretAudio"));
	TurretAudio->AttachToComponent(MeshComponent, FAttachmentTransformRules::KeepRelativeTransform);

	TeamFactionComponent = CreateDefaultSubobject<UTeamFactionComponent>(TEXT("TeamFactionComponent"));

	TargetFinderComponent = CreateDefaultSubobject<UTargetFinderComponent>(TEXT("TargetFinderComponent"));
	TargetFinderComponent->AddClassFilter(AAircraft::StaticClass());
	TargetFinderComponent->FindTargetPerFrame = true;

	ShooterComponent = CreateDefaultSubobject<UShooterComponent>(TEXT("ShooterComponent"));

	CurrentWeaponIndex = 0;

	TurretRotationFactor = 1.5f;
	TurretRotationErrorTolerance = 5.f;
}

void AAircraft::BeginPlay()
{
	Super::BeginPlay();

	TargetFinderComponent->OnTargetSearch.AddDynamic(this, &AAircraft::OnTargetSearchUpdate);

	DefaultPitchMin = PitchMin;
	DefaultPitchMax = PitchMax;
	DefaultYawMin = YawMin;
	DefaultYawMax = YawMax;
}

void AAircraft::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_DeltaTime = DeltaTime;


	if (UserController == nullptr) 
	{
		Shoot();
	}
}

bool AAircraft::ShouldStopVehicle()
{
	if (TargetActor && StopOnTargetFound)
	{
		return true;
	}
	else if (!TargetActor && StopOnTargetFound) 
	{
		VehiclePathFollowerComponent->ResumePath();
	}

	return Super::ShouldStopVehicle();
}

void AAircraft::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	Super::OnHealthUpdate(InHealthParameters);

	if (!HealthComponent->IsAlive())
	{
		TargetFinderComponent->SetFindTargetPerFrame(false);
		ShooterComponent->EndFireTimer();
	}
}

void AAircraft::OnTargetSearchUpdate(FTargetSearchParameters TargetSearchParameters)
{
	TargetSearchParams = TargetSearchParameters;

	TargetActor = TargetSearchParameters.TargetActor;

	if (!TargetActor) {
		VehiclePathFollowerComponent->ResumeNormalSpeed();
		ShooterComponent->StopFiringWeapons();
		GetWorldTimerManager().ClearTimer(THandler_RandomChangeWeapon);
		return;
	}

	// slowdown when target actor is found.
	if (SlowdownOnTargetFound) {
		VehiclePathFollowerComponent->Slowdown();
	}

	bool NewWeaponSet = false;

	if (THandler_RandomChangeWeapon.IsValid())
	{
		for (auto VehicleWeapon : VehicleWeapons)
		{
			if (NewWeaponSet) {
				break;
			}

			for (auto PreferredClassTarget : VehicleWeapon.PreferredClassTargets)
			{
				if (TargetActor->IsA(PreferredClassTarget) || TargetActor->GetParentActor() && TargetActor->GetParentActor()->IsA(PreferredClassTarget))
				{
					NewWeaponSet = UpdateCurrentWeapon(VehicleWeapon);

					if (NewWeaponSet)
					{
						break;
					}
				}
			}
		}
	}



	if (!NewWeaponSet)
	{
		// add more character to tank by radnomly changing weapons at random times
		if (!THandler_RandomChangeWeapon.IsValid())
		{
			// Change to secondary weapons at first then randomly change the weapons a few x seconds later
			ChangeSecondaryWeapon();

			GetWorldTimerManager().SetTimer(THandler_RandomChangeWeapon, this, &AAircraft::RandomChangeWeapon, FMath::RandRange(5.f, 10.f), true);
		}
	}
}

AMountedGun* AAircraft::SpawnVehicleWeapon(FVehicleWeapon VehicleWeapon)
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

void AAircraft::ChangeSecondaryWeapon()
{
	if (VehicleWeapons.Num() <= 0) {
		return;
	}

	for (int i = CurrentWeaponIndex; i < VehicleWeapons.Num(); i++)
	{
		auto VehicleWeapon = VehicleWeapons[i];

		// ignore weapons that are currently in use.
		if (VehicleWeapon.Weapon && !IsWeaponInUse(VehicleWeapon))
		{
			bool IsUpdated = UpdateCurrentWeapon(VehicleWeapon);

			if (IsUpdated)
			{
				CurrentWeaponIndex = i;

				// new weapon can be in use, no need to go further in this method.
				return;
			}

		}
	}

	// if this line of code is run, it means the current weapon index has reached its array size limit so go back to first index.
	CurrentWeaponIndex = 0;
	UpdateCurrentWeapon(VehicleWeapons[CurrentWeaponIndex]);

}

void AAircraft::RandomChangeWeapon()
{
	auto RandomBool = UKismetMathLibrary::RandomBool();

	if (RandomBool)
	{
		//UpdateCurrentWeapon(MainWeapon, VehicleWeaponMain);
	}
	else
	{
		ChangeSecondaryWeapon();
	}
}

FRotator AAircraft::FaceTarget(AActor* Actor, FRotator& TargetRotation)
{
	if (!Actor) {
		TargetRotation = FRotator::ZeroRotator;
		return UKismetMathLibrary::RLerp(RotationInput, TargetRotation, m_DeltaTime * TurretRotationFactor, false);
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	GetActorEyesViewPoint(EyeLocation, EyeRotation); // Grab the camera view points, this is a fail safe if vehicle weapons do not exist for some reason


	for (auto VehicleWeapon : CurrentVehicleWeapons)
	{
		// eye location should be retrieved from weapon socket location as the follow camera shouldn't change as it can be used by another mechanic such as orbiting around the vehicle
		// check if weapon class was provided as we cannot access the weapon object pointer when it has UPROPERTY attribute
		if (VehicleWeapon.Weapon)
		{
			EyeLocation = MeshComponent->GetSocketLocation(VehicleWeapon.WeaponSocketName);
		}
	}



	auto TargetLocation = TargetSearchParams.TargetLocation - EyeLocation;
	auto RootBone = MeshComponent->GetBoneName(0);
	auto TargetDirectionInvert = UKismetMathLibrary::InverseTransformDirection(MeshComponent->GetSocketTransform(RootBone), TargetLocation);
	TargetRotation = UKismetMathLibrary::MakeRotFromX(TargetDirectionInvert);

	return UKismetMathLibrary::RInterpTo(RotationInput, TargetRotation, GetWorld()->DeltaTimeSeconds, TurretRotationFactor);
}

void AAircraft::Shoot()
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

void AAircraft::ChangePitchValue(float InYawValue)
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

bool AAircraft::UpdateCurrentWeapon(FVehicleWeapon InVehicleWeapon)
{
	if (IsWeaponInUse(InVehicleWeapon)) {
		return false;
	}

	TArray<AWeapon*> Weapons;

	CurrentVehicleWeapons.Empty();

	if (InVehicleWeapon.Weapon) {
		Weapons.Add(InVehicleWeapon.Weapon);
		CurrentVehicleWeapons.Add(InVehicleWeapon);
	}

	for (auto Index : InVehicleWeapon.TwinWeaponIndexes)
	{
		if (VehicleWeapons[Index].Weapon && !Weapons.Contains(VehicleWeapons[Index].Weapon))
		{
			Weapons.Add(VehicleWeapons[Index].Weapon);
			CurrentVehicleWeapons.Add(VehicleWeapons[Index]);
		}
	}

	CurrentVehicleWeapon = InVehicleWeapon;

	ShooterComponent->SetWeapons(Weapons);

	return true;
}

bool AAircraft::IsWeaponInUse(FVehicleWeapon InVehicleWeapon)
{
	TArray<AWeapon*> Weapons;

	for (auto CurrentVehicleWpn : CurrentVehicleWeapons)
	{
		if (CurrentVehicleWpn.Weapon)
		{
			Weapons.Add(CurrentVehicleWpn.Weapon);
		}
	}

	return Weapons.Contains(InVehicleWeapon.Weapon);
}

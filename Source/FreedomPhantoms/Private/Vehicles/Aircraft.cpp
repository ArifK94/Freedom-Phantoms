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

	if (!HealthComponent->IsAlive())
	{
		return;
	}


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
	if (UKismetMathLibrary::RandomBool())
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

	FVector MidpointLocation = FVector::ZeroVector;
	int ValidWeapons = 0;

	for (auto VehicleWeapon : CurrentVehicleWeapons)
	{
		if (IsValid(VehicleWeapon.Weapon))
		{
			FVector WeaponLocation = MeshComponent->GetSocketLocation(VehicleWeapon.WeaponSocketName);
			MidpointLocation += WeaponLocation;
			ValidWeapons++;
		}
	}

	if (ValidWeapons > 0)
	{
		MidpointLocation /= ValidWeapons; // Calculate the average midpoint
	}
	else
	{
		// Fallback to actor location if no valid weapons
		FVector EyeLocation;
		FRotator EyeRotation;
		GetActorEyesViewPoint(EyeLocation, EyeRotation);
		MidpointLocation = EyeLocation;
	}

	auto TargetLocation = TargetSearchParams.TargetLocation - MidpointLocation;
	auto RootBone = MeshComponent->GetBoneName(0);
	auto TargetDirectionInvert = UKismetMathLibrary::InverseTransformDirection(MeshComponent->GetSocketTransform(RootBone), TargetLocation);
	TargetRotation = UKismetMathLibrary::MakeRotFromX(TargetDirectionInvert);

	return UKismetMathLibrary::RInterpTo(RotationInput, TargetRotation, m_DeltaTime, TurretRotationFactor);
}


void AAircraft::Shoot()
{
	// Get the target rotation
	FRotator TargetRotation = FRotator::ZeroRotator;
	FRotator NewRotationInput = FaceTarget(TargetActor, TargetRotation);


	if (!TargetActor)
	{
		// No target actor, stop firing and handle audio
		HandleFiring(false);
		HandleTurretAudio(false);
		return;
	}

	// Check if Pitch and Yaw are nearly equal, ignoring Roll
	// Normalize the yaw values to handle wrap-around
	float NormalizedTargetYaw = FMath::UnwindDegrees(TargetRotation.Yaw);
	float NormalizedInputYaw = FMath::UnwindDegrees(RotationInput.Yaw);

	// Check if Pitch and Yaw are nearly equal, ignoring Roll
	bool bIsNearlyFacing = FMath::Abs(RotationInput.Pitch - TargetRotation.Pitch) <= TurretRotationErrorTolerance &&
		FMath::Abs(NormalizedInputYaw - NormalizedTargetYaw) <= TurretRotationErrorTolerance;

	// Handle firing and audio based on whether the actor is facing the target
	HandleFiring(bIsNearlyFacing);
	HandleTurretAudio(!bIsNearlyFacing);  // Play turning sound if not facing the target

	SetRotationInput(NewRotationInput);
}

void AAircraft::HandleFiring(bool bIsFacingTarget)
{
	if (bIsFacingTarget)
	{
		ShooterComponent->BeginFire();
	}
	else
	{
		ShooterComponent->StopFiringWeapons();
	}
}

void AAircraft::HandleTurretAudio(bool bIsTurning)
{
	// Stop turret sound if it’s turning to the target
	if (bIsTurning)
	{
		TurretAudio->Sound = TurretTurnSound;
		if (!TurretAudio->IsPlaying())
		{
			TurretAudio->Play();
		}
	}
	else
	{
		TurretAudio->Stop();
		TurretAudio->Sound = TurretTurnStopSound;
		TurretAudio->Play();
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

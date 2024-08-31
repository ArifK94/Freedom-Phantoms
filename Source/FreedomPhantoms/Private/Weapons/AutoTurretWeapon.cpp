// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/AutoTurretWeapon.h"
#include "CustomComponents/ShooterComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "Weapons/MountedGun.h"
#include "Managers/DatatableManager.h"

#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"

AAutoTurretWeapon::AAutoTurretWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionProfileName(TEXT("BlockAll"));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(MeshComp);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetRegenerateHealth(false);

	TurretAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("TurretAudio"));
	TurretAudio->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepRelativeTransform);

	TeamFactionComponent = CreateDefaultSubobject<UTeamFactionComponent>(TEXT("TeamFactionComponent"));

	TargetFinderComponent = CreateDefaultSubobject<UTargetFinderComponent>(TEXT("TargetFinderComponent"));

	ShooterComponent = CreateDefaultSubobject<UShooterComponent>(TEXT("ShooterComponent"));

	TurretRotationFactor = 1.5f;
	TurretRotationErrorTolerance = 5.f;

	SurfaceImpactRowName = "Vehicle_Destruction";
}

void AAutoTurretWeapon::BeginPlay()
{
	Super::BeginPlay();

	SurfaceImpactSet = UDatatableManager::RetrieveSurfaceImpactSet(GetWorld(), SurfaceImpactRowName);

	HealthComponent->OnHealthChanged.AddDynamic(this, &AAutoTurretWeapon::OnHealthUpdate);

	TargetFinderComponent->OnTargetSearch.AddDynamic(this, &AAutoTurretWeapon::OnTargetSearchUpdate);

	SpawnVehicleWeapons();
}

void AAutoTurretWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_DeltaTime = DeltaTime;

	Shoot();
}

void AAutoTurretWeapon::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	if (!HealthComponent->IsAlive())
	{
		TargetFinderComponent->SetFindTargetPerFrame(false);
		ShooterComponent->EndFire();

		SetActorTickEnabled(false);

		// Destroy specified components
		for (int i = 0; i < DestroyableComponentList.Num(); i++)
		{
			auto Component = DestroyableComponentList[i];

			if (Component)
			{
				Component->DestroyComponent();
			}
		}

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


		// Play FX
		if (SurfaceImpactSet)
		{
			if (SurfaceImpactSet->NiagaraEffect)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SurfaceImpactSet->NiagaraEffect, GetActorLocation(), SurfaceImpactSet->VFXOffset.GetRotation().Rotator());
			}

			// Play explosion sound
			if (SurfaceImpactSet->Sound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurfaceImpactSet->Sound, GetActorLocation(), 1.0f, 1.0f, 0.0f, ExplosionAttenuation);
			}
		}

		if (ExplosionMesh)
		{
			MeshComp->SetSkeletalMesh(ExplosionMesh, false);
		}

		// Blast away nearby physics actors
		//RadialForceComp->FireImpulse();

		// Apply health damage
		//ApplyExplosionDamage(GetActorLocation(), InHealthParameters);
	}
}

void AAutoTurretWeapon::OnTargetSearchUpdate(FTargetSearchParameters TargetSearchParameters)
{
	TargetActor = TargetSearchParameters.TargetActor;

	if (!TargetActor) {
		ShooterComponent->EndFire();
		GetWorldTimerManager().ClearTimer(THandler_RandomChangeWeapon);
		return;
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

			GetWorldTimerManager().SetTimer(THandler_RandomChangeWeapon, this, &AAutoTurretWeapon::RandomChangeWeapon, FMath::RandRange(5.f, 10.f), true);
		}
	}
}

void AAutoTurretWeapon::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if (FollowCamera)
	{
		OutLocation = FollowCamera->GetComponentLocation();
		OutRotation = FollowCamera->GetComponentRotation();
	}
	else
	{
		OutLocation = MeshComp->GetComponentLocation();
		OutRotation = MeshComp->GetComponentRotation();
	}
}

void AAutoTurretWeapon::Shoot()
{
	if (!HealthComponent->IsAlive()) {
		return;
	}

	auto TargetRotation = FRotator::ZeroRotator;
	auto NewRotationInput = FaceTarget(TargetActor, TargetRotation);
	RotationInput = NewRotationInput;
	FollowCamera->SetRelativeRotation(RotationInput);

	auto NearlyEqualPitch = UKismetMathLibrary::NearlyEqual_FloatFloat(RotationInput.Pitch, TargetRotation.Pitch, TurretRotationErrorTolerance);
	auto NearlyEqualYaw = UKismetMathLibrary::NearlyEqual_FloatFloat(RotationInput.Yaw, TargetRotation.Yaw, TurretRotationErrorTolerance);
	auto NearlyEqualRoll = UKismetMathLibrary::NearlyEqual_FloatFloat(RotationInput.Roll, TargetRotation.Roll, TurretRotationErrorTolerance);
	auto NearlyEqual = NearlyEqualPitch && NearlyEqualYaw && NearlyEqualRoll;

	for (AWeapon* Weapon : ShooterComponent->GetWeapons())
	{
		Weapon->SetTargetActor(TargetActor);
	}

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

		ShooterComponent->EndFire();
	}
}

FRotator AAutoTurretWeapon::FaceTarget(AActor* Actor, FRotator& TargetRotation)
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
			EyeLocation = MeshComp->GetSocketLocation(VehicleWeapon.WeaponSocketName);
		}
	}



	auto TargetLocation = Actor->GetActorLocation() - EyeLocation;
	auto RootBone = MeshComp->GetBoneName(0);
	auto TargetDirectionInvert = UKismetMathLibrary::InverseTransformDirection(MeshComp->GetSocketTransform(RootBone), TargetLocation);
	TargetRotation = UKismetMathLibrary::MakeRotFromX(TargetDirectionInvert);

	return UKismetMathLibrary::RInterpTo(RotationInput, TargetRotation, GetWorld()->DeltaTimeSeconds, TurretRotationFactor);
}

void AAutoTurretWeapon::ChangeSecondaryWeapon()
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

void AAutoTurretWeapon::RandomChangeWeapon()
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

bool AAutoTurretWeapon::UpdateCurrentWeapon(FVehicleWeapon InVehicleWeapon)
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

bool AAutoTurretWeapon::IsWeaponInUse(FVehicleWeapon InVehicleWeapon)
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

void AAutoTurretWeapon::SpawnVehicleWeapons()
{
	for (int i = 0; i < VehicleWeapons.Num(); i++)
	{
		auto VehicleWeapon = VehicleWeapons[i];
		auto MyOwner = GetOwner() ? GetOwner() : this;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = MyOwner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		VehicleWeapons[i].Weapon = GetWorld()->SpawnActor<AMountedGun>(VehicleWeapon.WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (VehicleWeapons[i].Weapon)
		{
			VehicleWeapons[i].Weapon->SetOwner(MyOwner);
			VehicleWeapons[i].Weapon->SetAdjustBehindMG(false);
			VehicleWeapons[i].Weapon->SetCanTraceInteraction(false);
			VehicleWeapons[i].Weapon->SetCanExit(false);

			VehicleWeapons[i].Weapon->SetPitchMin(VehicleWeapon.PitchMin);
			VehicleWeapons[i].Weapon->SetPitchMax(VehicleWeapon.PitchMax);
			VehicleWeapons[i].Weapon->SetYawMin(VehicleWeapon.YawMin);
			VehicleWeapons[i].Weapon->SetYawMax(VehicleWeapon.YawMax);

			VehicleWeapons[i].Weapon->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleWeapon.WeaponSocketName);

			//VehicleWeapons[i].Weapon->OnKillConfirmed.AddDynamic(this, &AVehicleBase::OnWeaponKillConfirm);
		}

		auto VehicleWeaponPtr = new FVehicleWeapon();
		VehicleWeaponPtr->PitchMin = VehicleWeapon.PitchMin;
		VehicleWeaponPtr->PitchMax = VehicleWeapon.PitchMax;
		VehicleWeaponPtr->YawMin = VehicleWeapon.YawMin;
		VehicleWeaponPtr->YawMax = VehicleWeapon.YawMax;
		VehicleWeaponPtr->Weapon = VehicleWeapons[i].Weapon;
	}
}
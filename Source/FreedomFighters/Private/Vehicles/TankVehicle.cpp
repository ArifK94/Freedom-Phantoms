// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/TankVehicle.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"
#include "CustomComponents/ShooterComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/HealthComponent.h"

#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

void ATankVehicle::SetRotationInput(FRotator InRotation)
{
	//RotationInput.Pitch = FMath::ClampAngle(RotationInput.Pitch + InRotation.Pitch, PitchMin, PitchMax);
	//RotationInput.Pitch = FRotator::ClampAxis(RotationInput.Pitch);

	//RotationInput.Yaw = FMath::ClampAngle(RotationInput.Yaw + InRotation.Yaw, YawMin, YawMax);
	//RotationInput.Yaw = FRotator::ClampAxis(RotationInput.Yaw);

	RotationInput = InRotation;
}

void ATankVehicle::SetCurrentWeapon(AMountedGun* MountedGun)
{
	CurrentWeapon = MountedGun;
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
}

void ATankVehicle::BeginPlay()
{
	Super::BeginPlay();

	TargetFinderComponent->OnTargetSearch.AddDynamic(this, &ATankVehicle::OnTargetSearchUpdate);


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
		SetCurrentWeapon(MainWeapon);

	}
	else if (SecondaryWeapons.Num() > 0)
	{
		SetCurrentWeapon(SecondaryWeapons[0]);
	}




	//GetWorldTimerManager().SetTimer(THandler_Shoot, this, &ATankVehicle::Shoot, .2f, true);
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
		GetWorldTimerManager().ClearTimer(THandler_Shoot);
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
			SetCurrentWeapon(MainWeapon);
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

	if (CurrentWeapon) {
		CurrentWeapon->StopFire(); // stop firing current weapon before switching to another
	}

	SetCurrentWeapon(SecondaryWeapons[CurrentWeaponIndex]);
}

void ATankVehicle::RandomChangeWeapon()
{
	auto RandomBool = UKismetMathLibrary::RandomBool();

	if (RandomBool)
	{
		SetCurrentWeapon(MainWeapon);
	}
	else
	{
		ChangeSecondaryWeapon();
	}
}

FRotator ATankVehicle::FaceTarget(AActor* Actor)
{
	if (!Actor) {
		return  UKismetMathLibrary::RLerp(RotationInput, FRotator::ZeroRotator, m_DeltaTime * TurretRotationFactor, false);
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	GetActorEyesViewPoint(EyeLocation, EyeRotation);
	
	auto TargetLocation = Actor->GetActorLocation() - EyeLocation;

	auto RootBone = MeshComp->GetBoneName(0);
	auto TargetDirectionInvert = UKismetMathLibrary::InverseTransformDirection(MeshComp->GetSocketTransform(RootBone), TargetLocation);
	auto TargetRotation = UKismetMathLibrary::MakeRotFromX(TargetDirectionInvert);

	return UKismetMathLibrary::RLerp(RotationInput, TargetRotation, m_DeltaTime * TurretRotationFactor, false);

}

void ATankVehicle::Shoot()
{
	if (!HealthComp->IsAlive()) {
		return;
	}

	auto TargetRotation = FaceTarget(TargetActor);
	SetRotationInput(TargetRotation);

	if (TargetActor && UKismetMathLibrary::EqualEqual_RotatorRotator(RotationInput, TargetRotation))
	{
		ShooterComponent->BeginFire();
	}
	else
	{
		ShooterComponent->EndFire();
	}
}
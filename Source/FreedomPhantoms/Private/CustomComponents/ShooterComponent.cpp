// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/ShooterComponent.h"
#include "Weapons/Weapon.h"

#include "Kismet/KismetSystemLibrary.h"

UShooterComponent::UShooterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	m_TimeBetweenShotsMin = 1.5f;
	m_TimeBetweenShotsMax = 3.0f;

	FireRandomWeapon = false;
	HasFired = false;
}


void UShooterComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UShooterComponent::AddWeapon(AWeapon* Weapon)
{
	Weapons.Add(Weapon);
}

void UShooterComponent::SetWeapons(TArray<AWeapon*> InWeapons)
{
	// Stop firing current weapons
	EndFire();

	Weapons = InWeapons;
}

void UShooterComponent::BeginFire()
{
	// if no weapon assigned or timer already set
	if (THandler_BeginShoot.IsValid() || !GetOwner() || HasFired) {
		return;
	}

	HasFired = true;

	auto MyOwner = GetOwner();
	MyOwner->GetWorldTimerManager().ClearTimer(THandler_ResumeShoot);

	// fire weapons
	if (FireRandomWeapon)
	{
		auto RandomIndex = rand() % Weapons.Num();
		AWeapon* Weapon = Weapons[RandomIndex];
		Weapon->StartFire();
	}
	else 
	{
		for (auto Weapon : Weapons)
		{
			Weapon->StartFire();
		}
	}

	auto Delay = FMath::RandRange(m_TimeBetweenShotsMin, m_TimeBetweenShotsMax);

	MyOwner->GetWorldTimerManager().SetTimer(THandler_BeginShoot, this, &UShooterComponent::PauseFire, Delay, true, Delay); // stop firing after a random x secs
}

void UShooterComponent::PauseFire()
{
	// timer already set?
	if (THandler_ResumeShoot.IsValid()) {
		return;
	}

	EndFire();

	auto MyOwner = GetOwner();
	
	auto ResumeDelay = FMath::RandRange(m_TimeBetweenShotsMin, m_TimeBetweenShotsMax);

	OnShootPause.Broadcast(ResumeDelay);

	// Resume fire
	MyOwner->GetWorldTimerManager().SetTimer(THandler_ResumeShoot, this, &UShooterComponent::BeginFire, 1.f, true, ResumeDelay); // stop firing after a random x secs
}

void UShooterComponent::EndFire()
{
	if (!GetOwner()) {
		return;
	}
	auto MyOwner = GetOwner();
	
	MyOwner->GetWorldTimerManager().ClearTimer(THandler_ResumeShoot);
	MyOwner->GetWorldTimerManager().ClearTimer(THandler_BeginShoot);

	// stop firing weapons
	for (auto Weapon : Weapons)
	{
		if (UKismetSystemLibrary::IsValid(Weapon))
		{
			Weapon->StopFire();
		}
	}
	HasFired = false;
}
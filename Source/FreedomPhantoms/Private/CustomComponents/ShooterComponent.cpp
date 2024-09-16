// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/ShooterComponent.h"
#include "Weapons/Weapon.h"

UShooterComponent::UShooterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	m_TimeBetweenShotsMin = 1.5f;
	m_TimeBetweenShotsMax = 3.0f;

	FireRandomWeapon = false;
	bIsFiring = false;
	bIsPaused = false;

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
	EndFireTimer();
	StopFiringWeapons();

	Weapons = InWeapons;
}

void UShooterComponent::BeginFire()
{
	if (!bIsFiring && !bIsPaused)
	{
		bIsFiring = true;
		FireWeapon();
	}
}

void UShooterComponent::FireWeapon()
{
	AWeapon* SelectedWeapon = nullptr;

	// fire weapons
	if (FireRandomWeapon)
	{
		auto RandomIndex = rand() % Weapons.Num();

		if (Weapons.IsValidIndex(RandomIndex) && Weapons[RandomIndex]->CanFireWeapon())
		{
			SelectedWeapon = Weapons[RandomIndex];
			Weapons[RandomIndex]->StartFire();
		}
	}
	else
	{
		for (auto Weapon : Weapons)
		{
			if (IsValid(Weapon) && Weapon->CanFireWeapon())
			{
				SelectedWeapon = Weapon;
				Weapon->StartFire();
			}
		}
	}

	// If no available weapons to currently fire, then do not continue with the shooting functionality.
	if (!SelectedWeapon) {
		bIsFiring = false;
		return;
	}

	// stop firing after a random x secs
	float PauseDelay = FMath::RandRange(m_TimeBetweenShotsMin, m_TimeBetweenShotsMax);
	GetWorld()->GetTimerManager().SetTimer(PauseTimerHandle, this, &UShooterComponent::PauseFiring, PauseDelay, false);
}


void UShooterComponent::PauseFiring()
{
	bIsPaused = true;
	bIsFiring = false;

	StopFiringWeapons();

	auto ResumeDelay = FMath::RandRange(m_TimeBetweenShotsMin, m_TimeBetweenShotsMax);
	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &UShooterComponent::ResumeFiring, ResumeDelay, false);
	OnShootPause.Broadcast(ResumeDelay);
}

void UShooterComponent::ResumeFiring()
{
	bIsPaused = false;
	if (bIsFiring)
	{
		FireWeapon();
	}
}

void UShooterComponent::EndFireTimer()
{
	bIsFiring = false;
	bIsPaused = false;

	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(PauseTimerHandle);
}

void UShooterComponent::StopFiringWeapons()
{
	// stop firing weapons
	for (AWeapon* Weapon : Weapons)
	{
		if (IsValid(Weapon))
		{
			Weapon->StopFire();
		}
	}
}

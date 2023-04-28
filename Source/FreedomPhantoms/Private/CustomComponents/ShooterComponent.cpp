// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/ShooterComponent.h"
#include "Weapons/Weapon.h"

#include "Kismet/KismetSystemLibrary.h"

UShooterComponent::UShooterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	m_TimeBetweenShotsMin = 1.5f;
	m_TimeBetweenShotsMax = 3.0f;
}


void UShooterComponent::BeginPlay()
{
	Super::BeginPlay();
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
	if (THandler_BeginShoot.IsValid() || !GetOwner()) {
		return;
	}
	auto MyOwner = GetOwner();
	MyOwner->GetWorldTimerManager().ClearTimer(THandler_ResumeShoot);

	// fire weapons
	for (auto Weapon : Weapons)
	{
		Weapon->StartFire();
	}

	MyOwner->GetWorldTimerManager().SetTimer(THandler_BeginShoot, this, &UShooterComponent::PauseFire, 1.f, true, FMath::RandRange(m_TimeBetweenShotsMin, m_TimeBetweenShotsMax)); // stop firing after a random x secs
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
}
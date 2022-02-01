// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/ShooterComponent.h"
#include "Weapons/Weapon.h"

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

void UShooterComponent::SetWeapon(AWeapon* InWeapon)
{
	// Stop firing current weapon if already exists
	if (m_Weapon) {
		EndFire();
	}

	m_Weapon = InWeapon;
}

void UShooterComponent::BeginFire()
{
	// if no weapon assigned or timer already set
	if (!m_Weapon || THandler_BeginShoot.IsValid() || !GetOwner()) {
		return;
	}
	auto MyOwner = GetOwner();
	MyOwner->GetWorldTimerManager().ClearTimer(THandler_ResumeShoot);

	m_Weapon->StartFire(); // fire weapon
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
	if (!m_Weapon || !GetOwner()) {
		return;
	}
	auto MyOwner = GetOwner();
	
	MyOwner->GetWorldTimerManager().ClearTimer(THandler_ResumeShoot);
	MyOwner->GetWorldTimerManager().ClearTimer(THandler_BeginShoot);
	m_Weapon->StopFire(); // stop firing weapon
}
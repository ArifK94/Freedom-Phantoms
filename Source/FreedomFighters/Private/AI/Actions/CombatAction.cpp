// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/CombatAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/PumpActionWeapon.h"
#include "Weapons/ThrowableWeapon.h"
#include "Weapons/MountedGun.h"
#include "CustomComponents/MountedGunFinderComponent.h"
#include "StructCollection.h"
#include "Services/SharedService.h"

float UCombatAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);

	return .95f;
}

bool UCombatAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	return true;
}

bool UCombatAction::CanRunAsynchronously(AAIController* Controller, APawn* Pawn) const
{
	return true;
}

void UCombatAction::Spawn(AAIController* Controller, APawn* Pawn)
{
	Super::Spawn(Controller, Pawn);

	CombatAIController = Cast<ACombatAIController>(Controller);
	OwningCombatCharacter = Cast<ACombatCharacter>(Pawn);
}

void UCombatAction::Exit(AAIController* Controller, APawn* Pawn)
{
	Super::Exit(Controller, Pawn);

	OwningCombatCharacter->EndFire();

	// To avoid MG from contining to charge up.
	if (OwningCombatCharacter->IsUsingMountedWeapon() && OwningCombatCharacter->GetMountedGun()) {
		OwningCombatCharacter->EndAim();
	}
}

void UCombatAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	if (CombatAIController->GetEnemyActor()) {
		CombatMode();
	}
	else {
		EndShooting();
	}

	FaceTarget();
}

void UCombatAction::FaceTarget()
{
	if (CombatAIController->GetEnemyActor())
	{
		// if using a mounted gun
		if (OwningCombatCharacter->IsUsingMountedWeapon() && OwningCombatCharacter->GetMountedGun())
		{
			CombatAIController->GetMountedGunFinderComponent()->FocusTarget(OwningCombatCharacter->GetMountedGun(), CombatAIController->GetEnemyActor()->GetActorLocation());
		}
		else
		{
			//CombatAIController->SetFocus(CombatAIController->GetEnemyActor());
			CombatAIController->SetFocalPoint(CombatAIController->GetTargetSearchParams()->TargetLocation);
		}
	}
	else
	{
		if (OwningCombatCharacter->IsUsingMountedWeapon())
		{
			OwningCombatCharacter->GetMountedGun()->SetRotationInput(FRotator::ZeroRotator, 1.5f);
		}
	}
}

void UCombatAction::CombatMode()
{
	// set unlimited ammo
	if (!OwningCombatCharacter->GetCurrentWeapon()->GetHasUnlimitedAmmo()) {
		OwningCombatCharacter->GetCurrentWeapon()->SetUnlimitedAmmo(true);
	}

	// can only aim if not sprinting.
	if (!OwningCombatCharacter->IsAiming() && !OwningCombatCharacter->IsSprinting() && !OwningCombatCharacter->IsReloading()) {
		OwningCombatCharacter->BeginAim();
	}

	if (OwningCombatCharacter->GetCurrentWeapon()->GetCurrentAmmo() <= 0 || OwningCombatCharacter->IsReloading()) {
		ReloadWeapon();
	}
	else {

		if (!THandler_Shoot.IsValid()) {
			OwningCombatCharacter->GetWorldTimerManager().SetTimer(THandler_Shoot, this, &UCombatAction::ShootAtEnemy, 1.f, true);
		}

		//if (CanThrowGrenade()) {
		//	if (OwningCombatCharacter->GetCurrentWeapon() != OwningCombatCharacter->GetGrenadeWeapon()) {
		//		OwningCombatCharacter->EquipWeapon(OwningCombatCharacter->GetGrenadeWeapon());
		//	}
		//	else {
		//		ThrowGrenade();
		//	}
		//}
		//else {
		//	ShootAtEnemy();
		//}
	}
}

void UCombatAction::ShootAtEnemy()
{
	auto EnemyActor = CombatAIController->GetEnemyActor();

	if (EnemyActor == nullptr) {
		return;
	}

	if (OwningCombatCharacter->GetCurrentWeapon() == OwningCombatCharacter->GetGrenadeWeapon()) {
		OwningCombatCharacter->EquipWeapon(OwningCombatCharacter->GetPrimaryWeapon());
	}

	// check if enemy distance is close, if so then pull out pistol
	float DistanceDiff = FVector::Dist(OwningCombatCharacter->GetActorLocation(), EnemyActor->GetActorLocation());
	float randomDistanceLimit = FMath::RandRange(500.0f, 1000.0f);
	bool IsTargetClose = DistanceDiff < randomDistanceLimit;

	if (!IsTargetClose && OwningCombatCharacter->GetCurrentWeapon() == OwningCombatCharacter->GetSecondaryWeaponObj() // if target is not close, then switch back to primary
		&& !OwningCombatCharacter->GetIsInVehicle()) {
		OwningCombatCharacter->BeginWeaponSwap();
	}
	else { // Fire the weapon

		auto PumpActionWeapon = Cast<APumpActionWeapon>(OwningCombatCharacter->GetCurrentWeapon());

		// Shotguns require pump action rather than constant firing of weapon
		// check if using shotgun weapon type

		if (PumpActionWeapon) {
			if (PumpActionWeapon->GetHasLoadedShell()) {
				OwningCombatCharacter->BeginFire();
			}
			else {
				OwningCombatCharacter->EndFire();
			}
		}
		else {
			OwningCombatCharacter->BeginFire();
		}
	}

	// cooldown the shooting
	if (!THandler_EndShooting.IsValid()) {
		OwningCombatCharacter->GetWorldTimerManager().SetTimer(THandler_EndShooting, this, &UCombatAction::EndShooting, 1.f, true, FMath::RandRange(1.f, 2.f));
	}

}

void UCombatAction::EndShooting()
{
	OwningCombatCharacter->EndFire();

	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_Shoot);
	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_EndShooting);
}

void UCombatAction::ThrowGrenade()
{
	FRotator TargetRotation;
	bool IsReachable = SharedService::ThrowRotationAngle(OwningCombatCharacter->GetActorLocation(), CombatAIController->GetEnemyActor()->GetActorLocation(), TargetRotation);

	if (IsReachable) {
		OwningCombatCharacter->GetGrenadeWeapon()->SetVolleyAngle(TargetRotation);
		OwningCombatCharacter->BeginFire();
		CombatAIController->SetHasThrownGrenade(true);
	}
}

bool UCombatAction::CanThrowGrenade()
{
	if (CombatAIController->GetHasThrownGrenade()) {
		return false;
	}

	FRotator TargetRotation;
	bool IsReachable = SharedService::ThrowRotationAngle(OwningCombatCharacter->GetActorLocation(), CombatAIController->GetEnemyActor()->GetActorLocation(), TargetRotation);
	return IsReachable;
}

void UCombatAction::ReloadWeapon()
{
	OwningCombatCharacter->EndAim();

	// check if enemy distance is close, if so then pull out pistol
	float DistanceDiff = FVector::Dist(OwningCombatCharacter->GetActorLocation(), CombatAIController->GetEnemyActor()->GetActorLocation());
	float randomDistanceLimit = FMath::RandRange(500.0f, 1000.0f);
	bool IsTargetClose = DistanceDiff < randomDistanceLimit;

	auto PumpActionWeapon = Cast<APumpActionWeapon>(OwningCombatCharacter->GetCurrentWeapon());


	if (OwningCombatCharacter->GetCurrentWeapon()->GetCurrentAmmo() <= 0 && !PumpActionWeapon) { // Reload the weapon

		// if using primary weapon & enemy is nearby, then swap to secondary
		if (IsTargetClose &&
			OwningCombatCharacter->GetCurrentWeapon() != OwningCombatCharacter->GetSecondaryWeaponObj() && !OwningCombatCharacter->GetIsInVehicle()) {
			OwningCombatCharacter->BeginWeaponSwap();
		}
		else {
			OwningCombatCharacter->BeginReload();
		}
	}
	else if (PumpActionWeapon && (OwningCombatCharacter->GetCurrentWeapon()->GetCurrentAmmo() <= 0 || OwningCombatCharacter->IsReloading())) { // pump action weapons can be fired as soon as the first ammo shell is inserted, so better to add few more bullets first before firing again

		int RandomAmount = rand() % OwningCombatCharacter->GetCurrentWeapon()->getAmmoPerClip();

		if (OwningCombatCharacter->GetCurrentWeapon()->GetCurrentAmmo() <= 0 || OwningCombatCharacter->GetCurrentWeapon()->GetCurrentAmmo() < RandomAmount) {
			OwningCombatCharacter->BeginReload();
		}
		else {
			OwningCombatCharacter->EndReload();
		}
	}
	else {
		OwningCombatCharacter->EndReload();
	}
}

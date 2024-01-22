// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/CombatAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/PumpActionWeapon.h"
#include "Weapons/ThrowableWeapon.h"
#include "Weapons/MountedGun.h"
#include "StructCollection.h"
#include "Services/SharedService.h"
#include "CustomComponents/MountedGunFinderComponent.h"
#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/AIMovementComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

float UCombatAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);

	return .95f;
}

bool UCombatAction::CanRunAsynchronously(AAIController* Controller, APawn* Pawn) const
{
	return true;
}

bool UCombatAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	bool CanRun = true;

	if (CombatAIController->GetDisableCombat()) {
		CanRun = false;
	}


	if (OwningCombatCharacter->GetIsExitingVehicle()) {
		CanRun = false;
	}

	if (OwningCombatCharacter->GetIsInVehicle() && !OwningCombatCharacter->GetVehicletSeat().CanCharacterShoot) {
		CanRun = false;
	}

	if (!CanRun)
	{
		OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_Shoot);
		OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_EndShooting);

		OwningCombatCharacter->EndFire();
		OwningCombatCharacter->EndAim();

		return false;
	}

	return true;
}

void UCombatAction::Exit(AAIController* Controller, APawn* Pawn)
{
	Super::Exit(Controller, Pawn);

	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_Shoot);
	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_EndShooting);

	OwningCombatCharacter->EndFire();
	OwningCombatCharacter->EndAim();
}

void UCombatAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	// we do not want the AI to always be standing when throwing grenades.
	if (OwningCombatCharacter->GetCurrentWeapon() == OwningCombatCharacter->GetGrenadeWeapon())
	{
		if (OwningCombatCharacter->GetCharacterMovement()->IsCrouching())
		{
			OwningCombatCharacter->UnCrouch();
		}
	}

	if (CombatAIController->GetEnemyActor())
	{
		CombatMode();
	}
	else
	{
		EndShooting();
	}
}

void UCombatAction::CombatMode()
{
	// set unlimited ammo
	if (!OwningCombatCharacter->GetCurrentWeapon()->GetHasUnlimitedAmmo()) {
		OwningCombatCharacter->GetCurrentWeapon()->SetUnlimitedAmmo(true);
	}

	// reload the weapon
	if (OwningCombatCharacter->GetCurrentWeapon()->GetCurrentAmmo() <= 0 || OwningCombatCharacter->IsReloading())
	{
		ReloadWeapon();
	}
	else {

		// throw grenade
		if (CanThrowGrenade())
		{
			if (OwningCombatCharacter->GetCurrentWeapon() == OwningCombatCharacter->GetGrenadeWeapon())
			{
				ThrowGrenade();
			}
			else
			{
				OwningCombatCharacter->EquipWeapon(OwningCombatCharacter->GetGrenadeWeapon());
			}
		}
		// shoot with primary / secondary wepaons.
		else
		{
			Aim();

			// change grenade to primary weapon if greande is equipped.
			if (OwningCombatCharacter->GetCurrentWeapon() == OwningCombatCharacter->GetGrenadeWeapon())
			{
				OwningCombatCharacter->EquipWeapon(OwningCombatCharacter->GetPrimaryWeapon());
			}

			if (!THandler_Shoot.IsValid() && !THandler_EndShooting.IsValid()) {
				OwningCombatCharacter->GetWorldTimerManager().SetTimer(THandler_Shoot, this, &UCombatAction::ShootAtEnemy, 1.f, true);
			}
		}
	}
}

void UCombatAction::ShootAtEnemy()
{
	auto EnemyActor = CombatAIController->GetEnemyActor();

	if (EnemyActor == nullptr) {
		return;
	}

	// check if enemy distance is close, if so then pull out pistol
	bool IsTargetClose = USharedService::IsNearTargetPosition(OwningCombatCharacter, CombatAIController->GetEnemyActor(), CombatAIController->GetEnemyCloseRange());

	// if target is not close, then switch back to primary
	if (!IsTargetClose && OwningCombatCharacter->GetCurrentWeapon() == OwningCombatCharacter->GetSecondaryWeaponObj() && !OwningCombatCharacter->GetIsInVehicle())
	{
		OwningCombatCharacter->BeginWeaponSwap();
	}
	// Fire the weapon
	else
	{
		auto PumpActionWeapon = Cast<APumpActionWeapon>(OwningCombatCharacter->GetCurrentWeapon());

		// Shotguns require pump action rather than constant firing of weapon
		// check if using shotgun weapon type
		if (PumpActionWeapon)
		{
			if (PumpActionWeapon->GetHasLoadedShell())
			{
				OwningCombatCharacter->BeginFire();
			}
			else
			{
				OwningCombatCharacter->EndFire();
			}
		}
		else
		{
			OwningCombatCharacter->BeginFire();
		}
	}

	// cooldown the shooting
	if (!THandler_EndShooting.IsValid())
	{
		OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_Shoot);
		OwningCombatCharacter->GetWorldTimerManager().SetTimer(THandler_EndShooting, this, &UCombatAction::EndShooting, 1.f, true, FMath::RandRange(1.f, 2.f));
	}

}

void UCombatAction::EndShooting()
{
	if (OwningCombatCharacter->GetCurrentWeapon() == OwningCombatCharacter->GetGrenadeWeapon())
	{
		OwningCombatCharacter->EquipWeapon(OwningCombatCharacter->GetPrimaryWeapon());
	}


	OwningCombatCharacter->EndFire();

	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_EndShooting);
}


void UCombatAction::ReloadWeapon()
{
	// check if enemy distance is close, if so then pull out pistol
	bool IsTargetClose = USharedService::IsNearTargetPosition(OwningCombatCharacter, CombatAIController->GetEnemyActor(), CombatAIController->GetEnemyCloseRange());

	auto PumpActionWeapon = Cast<APumpActionWeapon>(OwningCombatCharacter->GetCurrentWeapon());

	// Reload the weapon
	if (OwningCombatCharacter->GetCurrentWeapon()->GetCurrentAmmo() <= 0 && !PumpActionWeapon)
	{
		// if using primary weapon & enemy is nearby, then swap to secondary
		// should not swap weapon while in cover, better to go back to cover and reload instead.
		if (IsTargetClose &&
			OwningCombatCharacter->GetCurrentWeapon() != OwningCombatCharacter->GetSecondaryWeaponObj() &&
			!OwningCombatCharacter->GetIsInVehicle() && !OwningCombatCharacter->IsTakingCover())
		{
			OwningCombatCharacter->BeginWeaponSwap();
		}
		else
		{
			OwningCombatCharacter->BeginReload();
		}
	}
	// pump action weapons can be fired as soon as the first ammo shell is inserted, so better to add few more bullets first before firing again
	else if (PumpActionWeapon && (OwningCombatCharacter->GetCurrentWeapon()->GetCurrentAmmo() <= 0 || OwningCombatCharacter->IsReloading()))
	{
		int RandomAmount = rand() % OwningCombatCharacter->GetCurrentWeapon()->getAmmoPerClip();

		if (OwningCombatCharacter->GetCurrentWeapon()->GetCurrentAmmo() <= 0 || OwningCombatCharacter->GetCurrentWeapon()->GetCurrentAmmo() < RandomAmount)
		{
			OwningCombatCharacter->BeginReload();
		}
		else
		{
			OwningCombatCharacter->EndReload();
		}
	}
	else
	{
		OwningCombatCharacter->EndReload();
	}
}

void UCombatAction::Aim()
{
	bool CanAim = true;

	bool CanBlindFire = CombatAIController->CanBlindCoverFire(OwningCombatCharacter->GetCurrentWeapon());

	// If taking cover, can the character perform blind fire?
	if (OwningCombatCharacter->IsTakingCover() && !CanBlindFire)
	{
		CanAim = false;
	}

	// If AI is moving to a location && 
	// If far away from target destination, then do not aim so character can sprint to destination.
	if (CombatAIController->GetAIMovementComponent()->GetCurrentMovement() == EPathFollowingRequestResult::RequestSuccessful && 
		!CombatAIController->IsNearTargetDestination())
	{
		CanAim = false;
	}

	if (CanAim)
	{
		OwningCombatCharacter->BeginAim();
	}
	else
	{
		OwningCombatCharacter->EndAim();
	}
}

void UCombatAction::ThrowGrenade()
{
	FRotator TargetRotation;
	bool IsReachable = USharedService::ThrowRotationAngle(OwningCombatCharacter->GetActorLocation(), CombatAIController->GetEnemyActor()->GetActorLocation(), TargetRotation);

	if (IsReachable)
	{
		OwningCombatCharacter->GetGrenadeWeapon()->SetVolleyAngle(TargetRotation);
		OwningCombatCharacter->BeginFire();

		if (OwningCombatCharacter->IsFiring()) {
			CombatAIController->SetHasThrownGrenade(true);
		}
	}
}

bool UCombatAction::CanThrowGrenade()
{
	if (CombatAIController->GetHasThrownGrenade()) {
		return false;
	}

	// is enemy close? if so, then do not throw grenade.
	if (USharedService::IsNearTargetPosition(OwningCombatCharacter, CombatAIController->GetEnemyActor(), CombatAIController->GetEnemyCloseRange())) {
		return false;
	}

	// has spent long on this enemy?
	if (!CombatAIController->GetHasTimeSpentOnEnemyReached()) {
		return false;
	}

	FRotator TargetRotation;
	bool IsReachable = USharedService::ThrowRotationAngle(OwningCombatCharacter->GetActorLocation(), CombatAIController->GetEnemyActor()->GetActorLocation(), TargetRotation);
	return IsReachable;
}
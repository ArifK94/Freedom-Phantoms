// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/CombatAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/PumpActionWeapon.h"
#include "Weapons/ThrowableWeapon.h"
#include "Weapons/MountedGun.h"
#include "CustomComponents/MountedGunFinderComponent.h"
#include "CustomComponents/TargetFinderComponent.h"
#include "StructCollection.h"
#include "Services/SharedService.h"

#include "Kismet/KismetMathLibrary.h"

float UCombatAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);

	return .95f;
}

bool UCombatAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	if (CombatAIController->GetDisableCombat()) {
		OwningCombatCharacter->EndFire();
		OwningCombatCharacter->EndAim();
		return false;
	}

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

	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_Shoot);
	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_EndShooting);

	OwningCombatCharacter->EndFire();

	// To avoid MG from contining to charge up.
	if (OwningCombatCharacter->IsUsingMountedWeapon() && OwningCombatCharacter->GetMountedGun()) {
		OwningCombatCharacter->EndAim();
	}
}

void UCombatAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	FaceTarget();

	if (CombatAIController->GetEnemyActor())
	{
		CombatMode();
	}
	else
	{
		EndShooting();
	}
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
			FVector TargetLocation;
			bool HasHitTarget = CombatAIController->GetTargetFinderComponent()->CanSeeTarget(CombatAIController->GetEnemyActor(), TargetLocation);

			if (HasHitTarget)
			{
				// use focal point since enemy maybe behind a barrier or cover so only the head would be visible.
				CombatAIController->SetFocalPosition(TargetLocation);
			}
			else
			{
				// face towards actor location in case enemy is taking cover.
				CombatAIController->SetFocus(CombatAIController->GetEnemyActor());
			}
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
	bool IsTargetClose = SharedService::IsNearTargetPosition(OwningCombatCharacter, CombatAIController->GetEnemyActor(), CombatAIController->GetEnemyCloseRange());

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
	OwningCombatCharacter->EndFire();

	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_EndShooting);
}


void UCombatAction::ReloadWeapon()
{
	OwningCombatCharacter->EndAim();

	// check if enemy distance is close, if so then pull out pistol
	bool IsTargetClose = SharedService::IsNearTargetPosition(OwningCombatCharacter, CombatAIController->GetEnemyActor(), CombatAIController->GetEnemyCloseRange());

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
	// always aim towards target :-
	// can only aim if not spritig,
	// if not in cover, since the character can do blind fire and no aim.
	if (!OwningCombatCharacter->IsAiming() && !OwningCombatCharacter->IsSprinting() && !OwningCombatCharacter->IsReloading() && !OwningCombatCharacter->IsTakingCover())
	{
		OwningCombatCharacter->BeginAim();
	}
	// can do blind fire if in cover without aiming but only if enemy is nearby.
	else if (SharedService::IsNearTargetPosition(OwningCombatCharacter, CombatAIController->GetEnemyActor(), CombatAIController->GetEnemyCloseRange()) &&
		OwningCombatCharacter->IsTakingCover())
	{
		if (!UKismetMathLibrary::RandomBool() && OwningCombatCharacter->IsAiming())
		{
			OwningCombatCharacter->EndAim();
		}
	}
}

void UCombatAction::ThrowGrenade()
{
	FRotator TargetRotation;
	bool IsReachable = SharedService::ThrowRotationAngle(OwningCombatCharacter->GetActorLocation(), CombatAIController->GetEnemyActor()->GetActorLocation(), TargetRotation);

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
	if (SharedService::IsNearTargetPosition(OwningCombatCharacter, CombatAIController->GetEnemyActor(), CombatAIController->GetEnemyCloseRange())) {
		return false;
	}

	// has spent long on this enemy?
	if (CombatAIController->GetTimeSpentOnEnemy() < CombatAIController->GetTimeSpentOnEnemyRange()) {
		return false;
	}

	FRotator TargetRotation;
	bool IsReachable = SharedService::ThrowRotationAngle(OwningCombatCharacter->GetActorLocation(), CombatAIController->GetEnemyActor()->GetActorLocation(), TargetRotation);
	return IsReachable;
}
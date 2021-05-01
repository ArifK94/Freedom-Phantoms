// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/CombatTaskNode.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Characters/CombatCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/Shotgun.h"


#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

EBTNodeResult::Type UCombatTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	UObject* EnemyObj = MyBlackboard->GetValueAsObject(BB_EnemyActor.SelectedKeyName);

	AActor* EnemyActor = Cast<AActor>(EnemyObj);

	if (EnemyActor == nullptr) {
		return EBTNodeResult::Failed;
	}

	AAIController* AIOwner = OwnerComp.GetAIOwner(); // to get the controller
	APawn* Pawn = AIOwner->GetPawn(); // to get the actor

	OwningCombatCharacter = Cast<ACombatCharacter>(Pawn);

	if (OwningCombatCharacter == nullptr) {
		return EBTNodeResult::Failed;
	}

	AWeapon* CurrentWeapon = OwningCombatCharacter->GetCurrentWeapon();

	if (CurrentWeapon == nullptr) {
		return EBTNodeResult::Failed;
	}

	// set unlimited ammo
	if (!OwningCombatCharacter->GetCurrentWeapon()->GetHasUnlimitedAmmo())
		OwningCombatCharacter->GetCurrentWeapon()->SetUnlimitedAmmo(true);

	if (OwningCombatCharacter->IsReloading())
	{
		OwningCombatCharacter->EndFire();
		OwningCombatCharacter->EndAim();
	}

	if (EnemyActor)
	{
		AIOwner->SetFocus(EnemyActor);

		OwningCombatCharacter->BeginAim();

		if (CurrentWeapon->getCurrentAmmo() <= 0)
		{
			// check if enemy distance is close, if so then pull out pistol
			// otherwise reload

			float PawnLocation = OwningCombatCharacter->GetActorLocation().Size();
			float EnemyLocation = EnemyActor->GetActorLocation().Size();

			float DistanceDiff = UKismetMathLibrary::Abs(PawnLocation - EnemyLocation);

			float randomDistanceLimit = FMath::RandRange(0.0f, 50.0f);

			if (DistanceDiff < randomDistanceLimit && CurrentWeapon != OwningCombatCharacter->GetSecondaryWeaponObj() && !OwningCombatCharacter->IsInHelicopter())
			{
				OwningCombatCharacter->EndFire();
				OwningCombatCharacter->EndAim();
				OwningCombatCharacter->BeginWeaponSwap();
			}
			else
			{
				OwningCombatCharacter->BeginReload();
			}
		}
		else
		{
			// Shotguns requires bolt action rather than constant firing of weapon
			// check if using shotgun weapon type
			AShotgun* ShotgunObj = Cast<AShotgun>(OwningCombatCharacter->GetCurrentWeapon());

			if (ShotgunObj)
			{
				if (ShotgunObj->GetHasLoadedShell())
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
				OwningCombatCharacter->GetWorldTimerManager().SetTimer(THandler_TimeBetweenShots, this, &UCombatTaskNode::EndFiring, FMath::RandRange(.1f, .5f), false, 0.0f);
			}
		}
	}
	else
	{
		AIOwner->ClearFocus(EAIFocusPriority::Gameplay);
		OwningCombatCharacter->EndFire();

		if (CurrentWeapon->getCurrentAmmo() <= 0) // reload clip if finished completely
		{
			OwningCombatCharacter->BeginReload();
		}
		else if (CurrentWeapon->getCurrentAmmo() < CurrentWeapon->getAmmoPerClip()) // reload if not on full clip
		{
			OwningCombatCharacter->BeginReload();
		}
		else
		{
			// switch back to primary
			if (CurrentWeapon == OwningCombatCharacter->GetSecondaryWeaponObj())
			{
				OwningCombatCharacter->BeginWeaponSwap();
			}
		}
	}
	return EBTNodeResult::Succeeded;
}

void UCombatTaskNode::EndFiring()
{
	OwningCombatCharacter->EndFire();
	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_TimeBetweenShots);
}
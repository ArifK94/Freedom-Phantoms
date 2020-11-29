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
	auto EnemyObj = MyBlackboard->GetValueAsObject(BB_EnemyActor.SelectedKeyName);


	AAIController* AIOwner = OwnerComp.GetAIOwner(); // to get the controller
	APawn* Pawn = AIOwner->GetPawn(); // to get the actor

	OwningCharacter = Cast<ACombatCharacter>(Pawn);

	if (OwningCharacter)
	{
		AWeapon* CurrentWeapon = OwningCharacter->GetCurrentWeapon();

		if (CurrentWeapon)
		{
			if (!OwningCharacter->GetCurrentWeapon()->GetHasUnlimitedAmmo())
				OwningCharacter->GetCurrentWeapon()->SetUnlimitedAmmo(true);

			if (OwningCharacter->IsReloading())
			{
				OwningCharacter->EndFire();
				OwningCharacter->EndAim();
			}


			if (EnemyObj)
			{
				AActor* EnemyActor = Cast<AActor>(EnemyObj);

				AIOwner->SetFocus(EnemyActor);

				OwningCharacter->BeginAim();

				if (CurrentWeapon->getCurrentAmmo() <= 0)
				{
					// check if enemy distance is close, if so then pull out pistol
					// otherwise reload

					float PawnLocation = Pawn->GetActorLocation().Size();
					float EnemyLocation = EnemyActor->GetActorLocation().Size();

					float DistanceDiff = UKismetMathLibrary::Abs(PawnLocation - EnemyLocation);

					float randomDistanceLimit = FMath::RandRange(800.0f, 1000.0f);

					if (DistanceDiff < randomDistanceLimit && CurrentWeapon != OwningCharacter->GetSecondaryWeaponObj())
					{
						OwningCharacter->EndFire();
						OwningCharacter->EndAim();
						OwningCharacter->BeginWeaponSwap();
					}
					else
					{
						CurrentWeapon->BeginReload();
					}
				}
				else
				{
					// Shotguns requires bolt action rather than constant firing of weapon
					// check if using shotgun weapon type 
					AShotgun* ShotgunObj = Cast<AShotgun>(OwningCharacter->GetCurrentWeapon());

					if (ShotgunObj)
					{
						if (ShotgunObj->HasLoadedShell())
						{
							OwningCharacter->BeginFire();
						}
						else
						{
							OwningCharacter->EndFire();
						}
					}
					else
					{
						OwningCharacter->BeginFire();
						//OwningCharacter->GetWorldTimerManager().SetTimer(THandler_TimeBetweenShots, this, &UCombatTaskNode::EndFiring, FMath::RandRange(1.0f, 3.0f), false, 0.0f);
					}
				}


			}
			else
			{

				if (CurrentWeapon->getCurrentAmmo() <= 0) // replenish clip if finished completely
				{
					ReloadWeapon();
				}
				else if (CurrentWeapon->getCurrentAmmo() < CurrentWeapon->getAmmoPerClip()) // replenish if not on full clip
				{
					OwningCharacter->GetWorldTimerManager().SetTimer(THandler_TimeReloadWeapon, this, &UCombatTaskNode::ReloadWeapon, FMath::RandRange(5.0f, 10.0f), false, 0.0f);
				}
				else
				{
					// switch back to primary
					if (CurrentWeapon == OwningCharacter->GetSecondaryWeaponObj())
					{
						OwningCharacter->BeginWeaponSwap();
					}
				}

				OwningCharacter->EndFire();
				OwningCharacter->EndAim();
				OwningCharacter->GetWorldTimerManager().ClearTimer(THandler_TimeBetweenShots);
			}

		}



	}
	return EBTNodeResult::Succeeded;

}


void UCombatTaskNode::EndFiring()
{
	OwningCharacter->EndFire();
	OwningCharacter->GetWorldTimerManager().ClearTimer(THandler_TimeBetweenShots);

}

void UCombatTaskNode::ReloadWeapon()
{
	OwningCharacter->BeginReload();
	OwningCharacter->GetWorldTimerManager().ClearTimer(THandler_TimeReloadWeapon);
}

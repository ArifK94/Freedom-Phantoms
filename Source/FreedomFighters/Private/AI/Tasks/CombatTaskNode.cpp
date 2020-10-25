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

		if (OwningCharacter->GetCurrentWeapon())
		{
			if (!OwningCharacter->GetCurrentWeapon()->GetHasUnlimitedAmmo())
				OwningCharacter->GetCurrentWeapon()->SetUnlimitedAmmo(true);
		}

		if (EnemyObj)
		{
			AActor* EnemyActor = Cast<AActor>(EnemyObj);

			AIOwner->SetFocus(EnemyActor);

			if (!OwningCharacter->IsReloading())
			{
				OwningCharacter->BeginAim();
			}
			else
			{
				OwningCharacter->EndFire();
			}


			// Shotguns requires bolt action rather than continious firing of weapon
			if (OwningCharacter->GetCurrentWeapon())
			{
				if (OwningCharacter->GetCurrentWeapon()->GetClass()->IsChildOf(AShotgun::StaticClass()))
				{
					// cast this cover actor to get the object
					auto ShotgunObj = Cast<AShotgun>(OwningCharacter->GetCurrentWeapon());

					if (ShotgunObj)
					{
						if (ShotgunObj->getCurrentAmmo() == 0)
						{
							ShotgunObj->BeginReload();
						}
						else
						{
							if (ShotgunObj->HasLoadedShell())
							{
								OwningCharacter->BeginFire();
							}
							else
							{
								EndFiring();
							}
						}

					}
				}
				else
				{
					OwningCharacter->BeginFire();
				}
			}


			return EBTNodeResult::Succeeded;

		}
		else
		{
			OwningCharacter->EndFire();
			OwningCharacter->EndAim();

		}
	}


	return EBTNodeResult::Succeeded;
}

void UCombatTaskNode::EndFiring()
{
	OwningCharacter->GetWorldTimerManager().ClearTimer(THandler_TimeBetweenShots);
	OwningCharacter->EndFire();
}

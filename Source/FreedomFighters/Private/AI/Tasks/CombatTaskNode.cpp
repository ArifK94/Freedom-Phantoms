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

UCombatTaskNode::UCombatTaskNode()
{
	TimeBetweenShots = FMath::RandRange(.1f, 4.0f);
}

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

			if (OwningCharacter->IsReloading())
			{
				OwningCharacter->EndFire();
				OwningCharacter->EndAim();
			}
			else
			{
				OwningCharacter->BeginAim();
			}



			// Shotguns requires bolt action rather than constant firing of weapon
			if (OwningCharacter->GetCurrentWeapon())
			{
				AWeapon* WeaponObj = OwningCharacter->GetCurrentWeapon();
				if (WeaponObj->getCurrentAmmo() <= 0)
				{
					// check if enemy distance is close, if so then pull out pistol
					// otherwise reload

					//float PawnLocation = Pawn->GetActorLocation().Size();
					//float EnemyLocation = EnemyActor->GetActorLocation().Size();

					//auto DistanceDiff = PawnLocation - EnemyLocation;

					//if (DistanceDiff < 1000.0f && OwningCharacter->GetCurrentWeapon() != OwningCharacter->GetSecondaryWeaponObj())
					//{
					//	OwningCharacter->EndFire();
					//	OwningCharacter->EndAim();
					//	OwningCharacter->swapWeapon();
					//}
					//else
					//{
					//	WeaponObj->BeginReload();
					//}

					WeaponObj->BeginReload();
				}
				else
				{
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
						//OwningCharacter->GetWorldTimerManager().SetTimer(THandler_TimeBetweenShots, OwningCharacter, &ACombatCharacter::BeginFire, 1.0f, false, 0.0f);
						OwningCharacter->BeginFire();
					}
				}
			}


			return EBTNodeResult::Succeeded;

		}
		else
		{
			OwningCharacter->EndFire();
			OwningCharacter->EndAim();
			OwningCharacter->GetWorldTimerManager().ClearTimer(THandler_TimeBetweenShots);
		}
	}


	return EBTNodeResult::Succeeded;
}

void UCombatTaskNode::EndFiring()
{
	OwningCharacter->BeginFire();


	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%f"), TimeBetweenShots));
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/CombatTaskNode.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Characters/CombatCharacter.h"

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
		if (EnemyObj)
		{
			AActor* EnemyActor = Cast<AActor>(EnemyObj);

			AIOwner->SetFocus(EnemyActor);

			OwningCharacter->BeginFire();

			return EBTNodeResult::Succeeded;

		}
		else
		{
			OwningCharacter->EndFire();
		}





		/*	switch (CombatMode)
			{
			case CombatState::Aiming:
				if (IsInCombat)
				{
					OwningCharacter->BeginAim();
				}
				else
				{
					OwningCharacter->EndAim();
				}

				return EBTNodeResult::Succeeded;
			case CombatState::Shooting:

				if (IsInCombat)
				{
					OwningCharacter->BeginFire();

					OwningCharacter->GetWorldTimerManager().SetTimer(THandler_TimeBetweenShots, this, &UCombatTaskNode::EndFiring, .1f, true, 0.0f);
				}
				else
				{
					OwningCharacter->EndFire();
				}

				return EBTNodeResult::Succeeded;
			default:
				return EBTNodeResult::Failed;
			}*/








	}


	return EBTNodeResult::Failed;
}

void UCombatTaskNode::EndFiring()
{
	OwningCharacter->GetWorldTimerManager().ClearTimer(THandler_TimeBetweenShots);
	OwningCharacter->EndFire();
}

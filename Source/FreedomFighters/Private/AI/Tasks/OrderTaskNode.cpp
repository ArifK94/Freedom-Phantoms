// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/OrderTaskNode.h"

#include "AIController.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"

UOrderTaskNode::UOrderTaskNode()
{
}



EBTNodeResult::Type UOrderTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	BlackboardComp = OwnerComp.GetBlackboardComponent();

	AAIController* AIOwner = OwnerComp.GetAIOwner(); // to get the controller
	APawn* Pawn = AIOwner->GetPawn(); // to get the actor

	ACombatCharacter* OwningCharacter = Cast<ACombatCharacter>(Pawn);

	if (OwningCharacter)
	{
		if (OwningCharacter->getCommander() != nullptr)
		{
			auto CommanderRecruit = OwningCharacter->getCommander()->GetRecruitInfo(Pawn);

			if (CommanderRecruit.Recruit == Pawn)
			{
				switch (CommanderRecruit.CurrentCommand)
				{
				case  CommanderOrders::Attack:
					AttackOrder(CommanderRecruit.TargetLocation);
					return EBTNodeResult::Succeeded;
				case CommanderOrders::Defend:
					DefendOrder(CommanderRecruit.TargetLocation);
					return EBTNodeResult::Succeeded;
				case CommanderOrders::Follow:
					CommanderRecruit.TargetLocation = OwningCharacter->getCommander()->GetActorLocation();
					FollowOrder(CommanderRecruit.TargetLocation);
					return EBTNodeResult::Succeeded;
				}
			}
		}
	}

	return EBTNodeResult::Failed;
}

void UOrderTaskNode::DefendOrder(FVector DefendLocation)
{
	BlackboardComp->SetValueAsVector(BB_TargetDestination.SelectedKeyName, DefendLocation);
}

void UOrderTaskNode::FollowOrder(FVector DefendLocation)
{
	BlackboardComp->SetValueAsVector(BB_TargetDestination.SelectedKeyName, DefendLocation);
}

void UOrderTaskNode::AttackOrder(FVector DefendLocation)
{
	BlackboardComp->SetValueAsVector(BB_TargetDestination.SelectedKeyName, DefendLocation);
}

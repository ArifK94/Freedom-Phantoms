// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/PriorityDestinationAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/AIMovementComponent.h"

float UPriorityDestinationAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);

	return .95f;
}

bool UPriorityDestinationAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	if (!CombatAIController->GetHasPriorityDestination()) {
		return false;
	}

	return true;
}

void UPriorityDestinationAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	MoveToResult = CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetPriorityLocation(), 10.f, AIBehaviourState::PriorityDestination);

	switch (MoveToResult)
	{
	case EPathFollowingRequestResult::Failed:
	case EPathFollowingRequestResult::AlreadyAtGoal:
		CombatAIController->SetHasPriorityDestination(false);
		break;
	default:
		break;
	}

}

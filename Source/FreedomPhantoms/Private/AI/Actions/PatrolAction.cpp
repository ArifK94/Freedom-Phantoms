// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/PatrolAction.h"
#include "Controllers/CombatAIController.h"
#include "CustomComponents/PatrolFollowerComponent.h"
#include "CustomComponents/AIMovementComponent.h"

float UPatrolAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);
	return .9f;
}

bool UPatrolAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	// do not patrol if there is an enemy presence
	if (CombatAIController->GetEnemyActor() || CombatAIController->GetLastSeenEnemyActor()) {
		IsPatrolling = false;
		MoveToResult = EPathFollowingRequestResult::Failed;
		return false;
	}

	if (!CombatAIController->GetPatrolFollowerComponent()->GetPatrolPath()) {
		IsPatrolling = false;
		MoveToResult = EPathFollowingRequestResult::Failed;
		return false;
	}

	return true;
}

void UPatrolAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	if (IsPatrolling)
	{
		// if reached current patrol point then move to next point.
		if (MoveToResult == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			MoveToNextPatrolPoint();
		}
	}
	else
	{
		StartPatrol();
	}
}

void UPatrolAction::StartPatrol()
{
	auto OutLocation = CombatAIController->GetPatrolFollowerComponent()->GetCurrentPathPoint();

	if (!OutLocation.IsZero())
	{
		CombatAIController->SetTargetDestination(OutLocation);
		MoveToResult = CombatAIController->GetAIMovementComponent()->MoveToDestination(OutLocation, 0.f, AIBehaviourState::Patrol, false);

		if (MoveToResult == EPathFollowingRequestResult::RequestSuccessful)
		{
			IsPatrolling = true;
		}
	}
}

void UPatrolAction::MoveToNextPatrolPoint()
{
	auto OutLocation = CombatAIController->GetPatrolFollowerComponent()->GetNextPathPoint();

	if (!OutLocation.IsZero())
	{
		CombatAIController->SetTargetDestination(OutLocation);
		MoveToResult = CombatAIController->GetAIMovementComponent()->MoveToDestination(OutLocation, 0.f, AIBehaviourState::Patrol, false);
	}

}

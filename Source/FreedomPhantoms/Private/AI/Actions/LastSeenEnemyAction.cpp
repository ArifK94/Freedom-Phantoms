// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/LastSeenEnemyAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/PatrolFollowerComponent.h"
#include "CustomComponents/AIMovementComponent.h"
#include "CustomComponents/AI/StrongholdDefenderComponent.h"

float ULastSeenEnemyAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);
	return .9f;
}

bool ULastSeenEnemyAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	// if defending stronghold then ignore this action.
	if (CombatAIController->GetStrongholdDefenderComponent()->GetStronghold()) {
		OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_LastSeenEnemy);
		return false;
	}

	// if an enemy is present then ignore this action.
	if (CombatAIController->GetEnemyActor()) {
		OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_LastSeenEnemy);
		return false;
	}

	// if last seen enemy does not exist then ignore this action.
	if (CombatAIController->GetLastSeenEnemyActor() == nullptr) {
		OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_LastSeenEnemy);
		return false;
	}

	return true;
}

void ULastSeenEnemyAction::Spawn(AAIController* Controller, APawn* Pawn)
{
	Super::Spawn(Controller, Pawn);

	Radius = FMath::RandRange(1000.f, 1500.f);
}

void ULastSeenEnemyAction::Exit(AAIController* Controller, APawn* Pawn)
{
	Super::Exit(Controller, Pawn);

	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_LastSeenEnemy);
}

void ULastSeenEnemyAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	// focus on last position.
	CombatAIController->SetFocalPosition(CombatAIController->GetLastSeenLocation());

	if (CombatAIController->GetMoveToLastSeenEnemy())
	{
		// aim weapon to stay alert.
		if (!OwningCombatCharacter->IsAiming())
		{
			OwningCombatCharacter->BeginAim();
		}

		if (MoveToResult == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			if (!THandler_LastSeenEnemy.IsValid())
			{
				OwningCombatCharacter->GetWorldTimerManager().SetTimer(THandler_LastSeenEnemy, this, &ULastSeenEnemyAction::RemoveLastSeen, 1.f, true, FMath::RandRange(5.f, 8.f));
			}
		}
		else
		{
			MoveToLastSeen();
		}
	}
	else
	{
		if (!THandler_LastSeenEnemy.IsValid())
		{
			OwningCombatCharacter->GetWorldTimerManager().SetTimer(THandler_LastSeenEnemy, this, &ULastSeenEnemyAction::RemoveLastSeen, 1.f, true, FMath::RandRange(5.f, 8.f));
		}
	}


}

void ULastSeenEnemyAction::MoveToLastSeen()
{
	CombatAIController->SetTargetDestination(CombatAIController->GetLastSeenLocation());
	MoveToResult = CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetLastSeenLocation(), Radius, AIBehaviourState::Patrol, false);
}

void ULastSeenEnemyAction::RemoveLastSeen()
{
	// Stop aiming if not combat alert
	if (!CombatAIController->GetStayCombatAlert())
	{
		OwningCombatCharacter->EndAim();
	}

	CombatAIController->SetLastSeenEnemyActor(nullptr);

	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_LastSeenEnemy);
}
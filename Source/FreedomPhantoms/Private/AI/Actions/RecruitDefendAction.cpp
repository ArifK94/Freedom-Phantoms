// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/RecruitDefendAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/AIMovementComponent.h"
#include "Services/SharedService.h"

#include "GameFramework/CharacterMovementComponent.h"

float URecruitDefendAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);
	return .9f;
}

bool URecruitDefendAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	// does AI have a commander?
	if (CombatAIController->GetCommander() == nullptr) {
		return false;
	}

	// is the current order the FOLLOW command?
	if (CombatAIController->GetCurrentCommand() != CommanderOrders::Defend) {
		return false;
	}

	// is searching or in cover? To allow AI to find cover once it has its reached order destination.
	if (CombatAIController->GetIsRunningForCover() || OwningCombatCharacter->IsTakingCover()) {
		return false;
	}

	if (CombatAIController->GetMoveToOrderResult() == EPathFollowingRequestResult::AlreadyAtGoal && MoveToRandomPointResult == EPathFollowingRequestResult::AlreadyAtGoal && IsCompelete) {
		return false;
	}

	return true;
}

void URecruitDefendAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	if (CombatAIController->GetMoveToOrderResult() == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		if (MoveToRandomPointResult == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			StayInRandomPoint();
		}
		else
		{
			if (HasFoundRandomPoint)
			{
				MoveToRandomPointResult = CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetTargetDestination(), 5.0f, AIBehaviourState::Normal);
			}
			else
			{
				FindRandomPoint();
			}
		}
	}
	else
	{
		MoveToDefend();
		HasFoundRandomPoint = false;
		IsCompelete = false;
	}
}

void URecruitDefendAction::MoveToDefend()
{
	auto TargetDest = CombatAIController->GetRecruitInfo()->TargetLocation;
	CombatAIController->SetTargetDestination(TargetDest);
	auto MoveToResult = CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetTargetDestination(), 5.0f, AIBehaviourState::PriorityOrdersCommander);
	CombatAIController->SetMoveToOrderResult(MoveToResult);
	OwningCombatCharacter->GetCharacterMovement()->bUseRVOAvoidance = false;
	MoveToRandomPointResult = EPathFollowingRequestResult::Failed;
}

void URecruitDefendAction::FindRandomPoint()
{
	// Find a random point around destination if has arrived at the original target destination
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(CombatAIController->GetCommander());

	auto NearDestination = CombatAIController->GetAIMovementComponent()->FindNearbyDestinationPoint(CombatAIController->GetTargetDestination(), 300.f, IgnoreActors);

	if (!NearDestination.IsZero())
	{
		CombatAIController->SetTargetDestination(NearDestination);
		HasFoundRandomPoint = true;
	}
}

void URecruitDefendAction::StayInRandomPoint()
{
	// Toggle crouch based on random possibility
	int RandonNum = FMath::RandRange(0, 1);

	if (RandonNum == 0)
	{
		OwningCombatCharacter->ToggleCrouch();
	}

	CombatAIController->SetStayCombatAlert(true);

	IsCompelete = true;
}
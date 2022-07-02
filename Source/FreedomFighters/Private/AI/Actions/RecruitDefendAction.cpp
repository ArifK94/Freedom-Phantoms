// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/RecruitDefendAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/AIMovementComponent.h"

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

	return true;
}

void URecruitDefendAction::Spawn(AAIController* Controller, APawn* Pawn)
{
	Super::Spawn(Controller, Pawn);

	CombatAIController = Cast<ACombatAIController>(Controller);
	OwningCombatCharacter = Cast<ACombatCharacter>(Pawn);
}

void URecruitDefendAction::Enter(AAIController* Controller, APawn* Pawn)
{
	Super::Enter(Controller, Pawn);
}

void URecruitDefendAction::Exit(AAIController* Controller, APawn* Pawn)
{
	Super::Exit(Controller, Pawn);
}

void URecruitDefendAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	MoveToDefend();
}

void URecruitDefendAction::MoveToDefend()
{
	auto TargetDest = CombatAIController->GetRecruitInfo()->TargetLocation;
	CombatAIController->SetTargetDestination(TargetDest);
	CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetTargetDestination(), .0f, AIBehaviourState::PriorityOrdersCommander);
	OwningCombatCharacter->GetCharacterMovement()->bUseRVOAvoidance = false;
}
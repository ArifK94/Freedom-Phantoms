// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/CoverAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/AIMovementComponent.h"
#include "CustomComponents/CoverFinderComponent.h"
#include "Services/SharedService.h"

#include "GameFramework/CharacterMovementComponent.h"

float UCoverAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);
	return .8f;
}

bool UCoverAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	// if not near to target destination, then do not search for cover.
	if (!SharedService::IsNearTargetPosition(OwningCombatCharacter->GetActorLocation(), CombatAIController->GetTargetDestination(), 10.f)) {
		return false;
	}

	return true;
}

void UCoverAction::Spawn(AAIController* Controller, APawn* Pawn)
{
	Super::Spawn(Controller, Pawn);

	CombatAIController = Cast<ACombatAIController>(Controller);
	OwningCombatCharacter = Cast<ACombatCharacter>(Pawn);
}

void UCoverAction::Enter(AAIController* Controller, APawn* Pawn)
{
	Super::Enter(Controller, Pawn);
}

void UCoverAction::Exit(AAIController* Controller, APawn* Pawn)
{
	Super::Exit(Controller, Pawn);
}

void UCoverAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	if (OwningCombatCharacter->IsTakingCover()) {
		Peak();
	}
	else if (!CoverFound) {
		FindCover();
	}
	else if (IsCoverValid) {

		if (CombatAIController->GetCoverFinderComponent()->IsCoverPointTaken(CoverLocation)) {
			IsCoverValid = false;
			CoverFound = false;
		}
		else {
			TakeCover();
		}

	}
}

void UCoverAction::FindCover()
{
	auto TargetLocation = OwningCombatCharacter->GetActorLocation();

	if (CombatAIController->GetEnemyActor()) {
		TargetLocation = CombatAIController->GetEnemyActor()->GetActorLocation();
	}

	CoverLocation = CombatAIController->GetCoverFinderComponent()->FindCover(CombatAIController->GetEnemyActor()->GetActorLocation());

	if (!CombatAIController->GetCoverFinderComponent()->IsCoverPointTaken(CoverLocation)) {
		CombatAIController->SetTargetDestination(CoverLocation);
		CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetTargetDestination(), .0f, AIBehaviourState::PriorityOrdersCommander);
		IsCoverValid = true;
	}

}

void UCoverAction::TakeCover()
{
	if (!SharedService::IsNearTargetPosition(OwningCombatCharacter->GetActorLocation(), CoverLocation, 10.f)) {
		return;
	}

	if (!OwningCombatCharacter->GetCharacterMovement()->IsCrouching())
		OwningCombatCharacter->ToggleCrouch();
}

void UCoverAction::Peak()
{

}

void UCoverAction::BeginCoverPeak()
{
	if (!OwningCombatCharacter->IsAtCoverCorner()) {
		return;
	}


	if (OwningCombatCharacter->IsFacingCoverRHS())
	{
		OwningCombatCharacter->CoverMovement(1.0f);
	}
	else
	{
		OwningCombatCharacter->CoverMovement(-1.0f);
	}
}

void UCoverAction::EndCoverPeak()
{
	OwningCombatCharacter->SetRightInputValue(.0f);
}
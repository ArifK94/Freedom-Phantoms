// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/RecruitFollowAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/AIMovementComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

float URecruitFollowAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);
	return .96f;
}

bool URecruitFollowAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	// does AI have a commander?
	if (CombatAIController->GetCommander() == nullptr) {
		return false;
	}

	// is the current order the FOLLOW command?
	if (CombatAIController->GetCurrentCommand() != CommanderOrders::Follow) {
		return false;
	}

	// if near commander then no need to run this action.
	if (CombatAIController->IsNearCommander()) {
		return false;
	}

	return true;
}

void URecruitFollowAction::Spawn(AAIController* Controller, APawn* Pawn)
{
	Super::Spawn(Controller, Pawn);

	CombatAIController = Cast<ACombatAIController>(Controller);
	OwningCombatCharacter = Cast<ACombatCharacter>(Pawn);
}

void URecruitFollowAction::Enter(AAIController* Controller, APawn* Pawn)
{
	Super::Enter(Controller, Pawn);

}

void URecruitFollowAction::Exit(AAIController* Controller, APawn* Pawn)
{
	Super::Exit(Controller, Pawn);

}

void URecruitFollowAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	MoveToCommander();
}

void URecruitFollowAction::MoveToCommander()
{
	if (CombatAIController->GetCommander()->GetCharacterMovement()->IsCrouching()) {
		if (!OwningCombatCharacter->GetCharacterMovement()->IsCrouching()) {
			OwningCombatCharacter->Crouch();
		}
	}
	else {
		if (OwningCombatCharacter->GetCharacterMovement()->IsCrouching()) {
			OwningCombatCharacter->UnCrouch();
		}
	}

	OwningCombatCharacter->DropMountedGun();
	CombatAIController->SetTargetDestination(CombatAIController->GetCommander()->GetActorLocation());
	CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetTargetDestination(), CombatAIController->GetAcceptanceRadius(), AIBehaviourState::PriorityOrdersCommander);
}
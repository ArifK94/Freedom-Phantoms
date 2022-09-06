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

void UPriorityDestinationAction::Spawn(AAIController* Controller, APawn* Pawn)
{
	Super::Spawn(Controller, Pawn);

	CombatAIController = Cast<ACombatAIController>(Controller);
	OwningCombatCharacter = Cast<ACombatCharacter>(Pawn);
}

void UPriorityDestinationAction::Enter(AAIController* Controller, APawn* Pawn)
{
	Super::Enter(Controller, Pawn);
}

void UPriorityDestinationAction::Exit(AAIController* Controller, APawn* Pawn)
{
	Super::Exit(Controller, Pawn);
}

void UPriorityDestinationAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	MoveToResult = CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetPriorityLocation(), 10.f, AIBehaviourState::PriorityDestination);

	switch (MoveToResult)
	{
	case EPathFollowingRequestResult::Failed:
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("%s"), *CombatAIController->GetPriorityLocation().ToString()));
		break;
	case EPathFollowingRequestResult::AlreadyAtGoal:
		//CombatAIController->SetHasPriorityDestination(false);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::Printf(TEXT("AlreadyAtGoal")));
		break;
	case EPathFollowingRequestResult::RequestSuccessful:
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("RequestSuccessful")));
		break;
	default:
		break;
	}

}

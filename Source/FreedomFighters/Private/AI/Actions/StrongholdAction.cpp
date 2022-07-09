// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/StrongholdAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/AIMovementComponent.h"
#include "CustomComponents/AI/StrongholdDefenderComponent.h"
#include "CustomComponents/CoverPointComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "Services/SharedService.h"

#include "GameFramework/CharacterMovementComponent.h"

float UStrongholdAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);

	// if defense point is of priority then no need for further action.
	//if (CombatAIController->GetStrongholdDefenderComponent()->GetChosenCoverPointComponent()) {

	//	if (CombatAIController->GetStrongholdDefenderComponent()->GetChosenCoverPointComponent()->GetIsPriority()) {
	//		return .5f;
	//	}
	//	else {
	//		return .8f;
	//	}
	//}

	return .9f;
}

bool UStrongholdAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	if (CombatAIController->GetStrongholdDefenderComponent()->GetStronghold() == nullptr) {
		return false;
	}

	// if defense point is of priority then no need for further action.
	if (CombatAIController->GetStrongholdDefenderComponent()->GetChosenCoverPointComponent() && CombatAIController->GetStrongholdDefenderComponent()->GetChosenCoverPointComponent()->GetIsPriority()) {

		// if near destination, then no need for further action
		if (SharedService::IsNearTargetPosition(OwningCombatCharacter->GetActorLocation(), CombatAIController->GetStrongholdDefenderComponent()->GetChosenCoverPointComponent()->GetComponentLocation(), 50.f)) {
			return false;
		}
	}

	return true;
}

void UStrongholdAction::Spawn(AAIController* Controller, APawn* Pawn)
{
	Super::Spawn(Controller, Pawn);

	CombatAIController = Cast<ACombatAIController>(Controller);
	OwningCombatCharacter = Cast<ACombatCharacter>(Pawn);
}

void UStrongholdAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	// Remove wounded ability for stronghold defenders.
	if (OwningCombatCharacter->GetHealthComp()->GetCanBeWounded()) {
		OwningCombatCharacter->GetHealthComp()->SetCanBeWounded(false);
	}


	if (CombatAIController->GetStrongholdDefenderComponent()->GetChosenCoverPointComponent()) {
		MoveToDefensePoint();
	}
	else {
		CombatAIController->GetStrongholdDefenderComponent()->FindDefenderPoint();
	}

}

void UStrongholdAction::MoveToDefensePoint()
{
	auto TargetDestination = CombatAIController->GetStrongholdDefenderComponent()->GetChosenCoverPointComponent()->GetComponentLocation();
	CombatAIController->SetTargetDestination(TargetDestination);
	CombatAIController->GetAIMovementComponent()->MoveToDestination(TargetDestination, 20.f, AIBehaviourState::Normal, true, false);
}
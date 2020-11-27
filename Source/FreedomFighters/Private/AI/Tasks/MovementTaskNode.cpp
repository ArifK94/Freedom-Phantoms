// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/MovementTaskNode.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "NavFilters/NavigationQueryFilter.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "Characters/BaseCharacter.h"

#include "Kismet/KismetMathLibrary.h"


UMovementTaskNode::UMovementTaskNode()
{
	AcceptanceRadius = 100.0f;
	DistanceDiffSprint = 200.0f;
}

EBTNodeResult::Type UMovementTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	FVector TargetDestination = MyBlackboard->GetValueAsVector(BB_TargetDestination.SelectedKeyName);

	AAIController* AIOwner = OwnerComp.GetAIOwner(); // to get the controller
	APawn* Pawn = AIOwner->GetPawn(); // to get the actor

	ABaseCharacter* OwningCharacter = Cast<ABaseCharacter>(Pawn);

	if (OwningCharacter)
	{
		FVector OwnerLocation = OwningCharacter->GetActorLocation();

		AIOwner->MoveToLocation(TargetDestination, AcceptanceRadius, StopOnOverlap, UsePathfinding, ProjectDestinationToNavigation, CanStrafe, FilterClass, AllowPartialPaths);

		float CurrentTargetDistance = UKismetMathLibrary::Vector_Distance(OwnerLocation, TargetDestination);

		if (CurrentTargetDistance > DistanceDiffSprint)
		{
			OwningCharacter->BeginSprint();
		}
		else
		{
			OwningCharacter->EndSprint();
		}
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}

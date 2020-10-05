// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/MovementTaskNode.h"
#include "AIController.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "Characters/BaseCharacter.h"

#include "Kismet/KismetMathLibrary.h"


UMovementTaskNode::UMovementTaskNode(const FObjectInitializer& ObjectInit)
{
	AcceptanceRadius = 50.0f;
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
	 auto MovementInput =	AIOwner->MoveToLocation(TargetDestination, AcceptanceRadius);


	 if (MovementInput == EPathFollowingRequestResult::AlreadyAtGoal)
	 {
		 OwningCharacter->SetForwardInput(0.0f);
	 }
	 else
	 {
		 OwningCharacter->SetForwardInput(1.0f);
	 }

		FVector OwnerLocation = OwningCharacter->GetActorLocation();

		float CurrentTargetDistance = UKismetMathLibrary::Vector_Distance(OwnerLocation, TargetDestination);

// 		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("CurrentTargetDistance: %f"), CurrentTargetDistance));


		if (CurrentTargetDistance > 200.0f)
		{
			OwningCharacter->BeginSprint();
		}
		else
		{
			OwningCharacter->EndSprint();
		}
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Succeeded!"));

		return EBTNodeResult::Succeeded;
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed!"));

	return EBTNodeResult::Failed;
}

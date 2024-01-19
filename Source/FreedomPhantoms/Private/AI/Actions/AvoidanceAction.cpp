// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/AvoidanceAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/AIMovementComponent.h"
#include "ObjectPoolActor.h"
#include "Services/SharedService.h"
#include "StructCollection.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"

float UAvoidanceAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);

	return 1.f;
}

bool UAvoidanceAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	auto AvoidableActor = CombatAIController->GetAvoidableParams().Actor;
	if (!UKismetSystemLibrary::IsValid(AvoidableActor)) {
		return false;
	}
	else {

		auto Poolable = Cast<AObjectPoolActor>(AvoidableActor);

		if (UKismetSystemLibrary::IsValid(Poolable)) {
			if (Poolable->GetIsDestroyed()) {
				return false;
			}
		}
	}

	return true;
}

void UAvoidanceAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	if (CombatAIController->GetAvoidableParams().Actor) {

		// is still near avoid actor?
		if (USharedService::IsNearTargetPosition(OwningCombatCharacter->GetActorLocation(), CombatAIController->GetAvoidableParams().Actor->GetActorLocation(), CombatAIController->GetAvoidableParams().AvoidableDistance)) {
			Flee();
		}
		else {
			CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetTargetDestination(), 20.f, AIBehaviourState::PriorityDestination);
		}
	}

}

void UAvoidanceAction::Flee()
{
	FVector OwnerLocation = OwningCombatCharacter->GetActorLocation();
	FVector AvoidableLocation = CombatAIController->GetAvoidableParams().Actor->GetActorLocation();

	// Distance away from the avoidable actor.
	FVector DirectionAvoidance = UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::FindLookAtRotation(OwnerLocation, AvoidableLocation));
	DirectionAvoidance = (DirectionAvoidance * (CombatAIController->GetAvoidableParams().AvoidableDistance * -1.f)) + OwnerLocation;


	// get a random reachable point away from avoidance distance to make the move to dynamic
	FNavLocation NavLocation;
	UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->GetRandomReachablePointInRadius(DirectionAvoidance, CombatAIController->GetAvoidableParams().AvoidableDistance, NavLocation);


	// move to the point away from the avoidable
	CombatAIController->SetTargetDestination(NavLocation.Location);
	CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetTargetDestination(), 20.f, AIBehaviourState::PriorityDestination);
}
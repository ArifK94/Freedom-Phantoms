// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/SelectEnemyService.h"
#include "Characters/CombatCharacter.h"

#include "AIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "CustomComponents/HealthComponent.h"

#include <array>

void USelectEnemyService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	float TargetSightDistance = 1000.0f;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	AAIController* AIOwner = OwnerComp.GetAIOwner(); // to get the controller
	APawn* Pawn = AIOwner->GetPawn(); // to get the actor


	ACombatCharacter* OwningCharacter = Cast<ACombatCharacter>(Pawn);
	AActor* SelectedTargetActor = nullptr;

	UAIPerceptionComponent* AISight = Cast<UAIPerceptionComponent>(Pawn->GetComponentByClass(UAIPerceptionComponent::StaticClass()));

	TArray<AActor*> ActorsInSight;

	AISight->GetKnownPerceivedActors(TSubclassOf<UAISense_Sight>(), ActorsInSight);


	for (int index = 0; index < ActorsInSight.Num(); index++)
	{
		AActor* CurrentActor = ActorsInSight[index];

		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(CurrentActor->GetComponentByClass(UHealthComponent::StaticClass()));

		bool IsAlive = CurrentHealth->getCurrentHealth() > 0.0f;
		bool IsEnemy = !UHealthComponent::IsFriendly(Pawn, CurrentActor);


		if (IsEnemy && IsAlive)
		{
			float PawnLocation = Pawn->GetActorLocation().Size();
			float EnemyLocation = CurrentActor->GetActorLocation().Size();

			auto DistanceDiff = PawnLocation - EnemyLocation;

			if (DistanceDiff < TargetSightDistance)
			{
				TargetSightDistance = DistanceDiff;

				SelectedTargetActor = CurrentActor;

				OwningCharacter->TargetFound();
			}
		}
	}

	if (SelectedTargetActor != nullptr)
	{
		BlackboardComp->SetValueAsObject(BB_TargetActor.SelectedKeyName, SelectedTargetActor);
	}
	else
	{
		BlackboardComp->SetValueAsObject(BB_TargetActor.SelectedKeyName, nullptr);
	}


}

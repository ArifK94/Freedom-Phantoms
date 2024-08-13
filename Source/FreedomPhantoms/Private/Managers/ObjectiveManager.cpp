// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/ObjectiveManager.h"
#include "Objectives/BaseObjective.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"

UObjectiveManager::UObjectiveManager()
{
	TotalObjectives = 0;
	TotalRequiredObjectives = 0;
}

UWorld* UObjectiveManager::GetWorld() const
{
	UObject* Outer = GetOuter();
	if (!Outer)
		return nullptr;
	AGameStateBase* GameStateBase = Cast<AGameStateBase>(GetOuter());
	if (!GameStateBase)
		return nullptr;

	return GameStateBase->GetWorld();
}

void UObjectiveManager::OnObjectiveCompleted(ABaseObjective* Objective)
{
	auto Index = Objectives.Find(Objective);
	Objectives.RemoveAt(Index);

	// if objective is required, then increment current count.
	if (!Objective->GetIsOptional())
	{
		CurrentRequiredObjectivesCompleted++;
	}

	// if reached total required objective count then the game can end.
	if (TotalRequiredObjectives > 0 && CurrentRequiredObjectivesCompleted >= TotalRequiredObjectives)
	{
		CurrentMissionObjective = nullptr;
		OnMissionCompleted.Broadcast(true);
	}
	// set the next objective
	else if (!Objectives.IsEmpty())
	{
		CurrentMissionObjective = Objectives[Objectives.Num() - 1];
	}
}

/**
* Reset certain variables as this function can be repeatedly called.
*/
void UObjectiveManager::LoadObjectives()
{
	TArray<AActor*> ObjectiveActors = GetObjectiveActors();

	TotalObjectives = ObjectiveActors.Num();

	for (int i = 0; i < TotalObjectives; i++)
	{
		if (Objectives.Contains(ObjectiveActors[i])) {
			continue;
		}

		ABaseObjective* Objective = Cast<ABaseObjective>(ObjectiveActors[i]);

		// increment the total required objectives.
		if (!Objective->GetIsOptional())
		{
			TotalRequiredObjectives++;
		}

		if (CurrentMissionObjective == nullptr)
		{
			CurrentMissionObjective = Objective;
		}

		Objective->OnObjectiveCompleted.AddDynamic(this, &UObjectiveManager::OnObjectiveCompleted);
		
		Objectives.Add(Objective);
	}
}

TArray<AActor*> UObjectiveManager::GetObjectiveActors()
{
	TArray<AActor*> ObjectiveActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseObjective::StaticClass(), ObjectiveActors);
	return ObjectiveActors;
}

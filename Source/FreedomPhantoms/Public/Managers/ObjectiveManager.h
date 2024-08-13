// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ObjectiveManager.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompletedSignature, bool, IsComplete);

class ABaseObjective;
UCLASS()
class FREEDOMPHANTOMS_API UObjectiveManager : public UObject
{
	GENERATED_BODY()
	
private:

	int TotalObjectives;
	int TotalRequiredObjectives;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<ABaseObjective*> Objectives;
	int CurrentRequiredObjectivesCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		ABaseObjective* CurrentMissionObjective;

public:
	UPROPERTY(BlueprintAssignable)
		FOnMissionCompletedSignature OnMissionCompleted;

public:
	UObjectiveManager();

	virtual UWorld* GetWorld() const override;

	UFUNCTION()
		void OnObjectiveCompleted(ABaseObjective* Objective);

	void LoadObjectives();

private:
	TArray<AActor*> GetObjectiveActors();

public:

	TArray<ABaseObjective*> GetObjectives() { return Objectives; }

	int GetTotalObjectives() { return TotalObjectives; }

	int GetTotalRequiredObjectives() { return TotalRequiredObjectives; }

	void SetCurrentMissionObjective(ABaseObjective* Objective) { CurrentMissionObjective = Objective; }

};

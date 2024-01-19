// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "RecruitFollowAction.generated.h"

/**
 * AI follow commander order task.
 */
UCLASS()
class FREEDOMPHANTOMS_API URecruitFollowAction : public UUtilityAIAction
{
	GENERATED_BODY()

public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:
	void MoveToCommander();
	
};

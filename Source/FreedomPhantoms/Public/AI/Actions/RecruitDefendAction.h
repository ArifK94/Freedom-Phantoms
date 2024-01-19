// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "RecruitDefendAction.generated.h"

/**
 * AI defend order from commander task.
 */
UCLASS()
class FREEDOMPHANTOMS_API URecruitDefendAction : public UUtilityAIAction
{
	GENERATED_BODY()
	
private:
	EPathFollowingRequestResult::Type MoveToRandomPointResult;

	bool HasFoundRandomPoint;
	bool IsCompelete;

public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:
	void MoveToDefend();

	void StayInRandomPoint();

	void FindRandomPoint();
};

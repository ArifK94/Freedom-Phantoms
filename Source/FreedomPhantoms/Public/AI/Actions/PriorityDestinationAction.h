// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "PriorityDestinationAction.generated.h"

/**
 * When AI needs to move to a top priority desintation regardless if following a commander. Useful for setting AI to get into position.
 */
UCLASS()
class FREEDOMPHANTOMS_API UPriorityDestinationAction : public UUtilityAIAction
{
	GENERATED_BODY()

private:
	mutable EPathFollowingRequestResult::Type MoveToResult;

public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;
	
};

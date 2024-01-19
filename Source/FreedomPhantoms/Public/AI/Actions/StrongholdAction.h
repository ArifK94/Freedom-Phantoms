// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "StrongholdAction.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMPHANTOMS_API UStrongholdAction : public UUtilityAIAction
{
	GENERATED_BODY()

private:
	EPathFollowingRequestResult::Type MoveToResult;

	/**
	* Has AI arrived at the stronghold point?
	*/
	bool ArrivedAtTarget;

public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:
	void MoveToDefensePoint();
	
};

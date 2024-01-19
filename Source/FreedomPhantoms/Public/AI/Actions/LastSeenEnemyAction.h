// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "LastSeenEnemyAction.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMPHANTOMS_API ULastSeenEnemyAction : public UUtilityAIAction
{
	GENERATED_BODY()
	
private:
	mutable EPathFollowingRequestResult::Type MoveToResult;

	mutable FTimerHandle THandler_LastSeenEnemy;

	// the radius to which to move to.
	float Radius;


public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	virtual void Spawn(AAIController* Controller, APawn* Pawn) override;

	virtual void Exit(AAIController* Controller, APawn* Pawn) override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:
	void MoveToLastSeen();

	void RemoveLastSeen();
};

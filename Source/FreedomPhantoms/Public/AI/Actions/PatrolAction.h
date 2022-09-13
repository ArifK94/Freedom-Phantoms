// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "PatrolAction.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMPHANTOMS_API UPatrolAction : public UUtilityAIAction
{
	GENERATED_BODY()

private:
	class ACombatAIController* CombatAIController;
	class ACombatCharacter* OwningCombatCharacter;

	mutable EPathFollowingRequestResult::Type MoveToResult;

	mutable bool IsPatrolling;

public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	virtual void Spawn(AAIController* Controller, APawn* Pawn) override;

	virtual void Enter(AAIController* Controller, APawn* Pawn) override;

	virtual void Exit(AAIController* Controller, APawn* Pawn) override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:
	void StartPatrol();

	void MoveToNextPatrolPoint();

};

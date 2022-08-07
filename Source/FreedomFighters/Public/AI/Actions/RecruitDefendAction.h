// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "RecruitDefendAction.generated.h"

/**
 * AI defend order from commander task.
 */
UCLASS()
class FREEDOMFIGHTERS_API URecruitDefendAction : public UUtilityAIAction
{
	GENERATED_BODY()
	
private:
	class ACombatAIController* CombatAIController;
	class ACombatCharacter* OwningCombatCharacter;

	EPathFollowingRequestResult::Type MoveToRandomPointResult;

	bool HasFoundRandomPoint;
	bool IsCompelete;

public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	virtual void Spawn(AAIController* Controller, APawn* Pawn) override;

	virtual void Enter(AAIController* Controller, APawn* Pawn) override;

	virtual void Exit(AAIController* Controller, APawn* Pawn) override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:
	void MoveToDefend();

	void StayInRandomPoint();

	void FindRandomPoint();
};

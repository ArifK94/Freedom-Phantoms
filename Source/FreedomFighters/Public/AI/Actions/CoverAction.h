// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "CoverAction.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMFIGHTERS_API UCoverAction : public UUtilityAIAction
{
	GENERATED_BODY()
	
private:
	class ACombatAIController* CombatAIController;
	class ACombatCharacter* OwningCombatCharacter;


	FVector CoverLocation;

	float PeakCountdown;

	EPathFollowingRequestResult::Type MoveToResult;

	FTimerHandle THandler_Peaking;
	FTimerHandle THandler_EndPeaking;

	// stance state time handler for when not find a cover e.g. crouch, stand etc.
	FTimerHandle THandler_Stance;
	FTimerHandle THandler_EndStance;


public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	virtual void Spawn(AAIController* Controller, APawn* Pawn) override;

	virtual void Enter(AAIController* Controller, APawn* Pawn) override;

	virtual void Exit(AAIController* Controller, APawn* Pawn) override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:
	void FindCover();

	void TakeCover();

	void BeginCoverPeak();

	void EndCoverPeak();

	void ChangeStance();

	void ClearStanceTimer();
};

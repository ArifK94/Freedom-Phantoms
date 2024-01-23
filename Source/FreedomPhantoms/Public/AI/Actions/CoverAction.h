// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "CoverAction.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMPHANTOMS_API UCoverAction : public UUtilityAIAction
{
	GENERATED_BODY()
	
private:
	float mDeltaTime;
	FTransform CoverPoint;
	float PeakCountdown;

	EPathFollowingRequestResult::Type MoveToResult;

	FTimerHandle THandler_Peaking;
	FTimerHandle THandler_EndPeaking;

	// stance state time handler for when not find a cover e.g. crouch, stand etc.
	FTimerHandle THandler_Stance;


public:
	UCoverAction();

	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	virtual void Exit(AAIController* Controller, APawn* Pawn) override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:
	void FindCover();

	void TakeCover();

	void BeginCoverPeak();

	void EndCoverPeak();

	void ChangeStance();
};

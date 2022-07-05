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

	bool CoverFound;
	bool IsCoverValid;

	FVector CoverLocation;

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

	void Peak();

	void BeginCoverPeak();

	void EndCoverPeak();
};

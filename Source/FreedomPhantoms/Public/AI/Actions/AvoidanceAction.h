// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "AvoidanceAction.generated.h"

/**
 * AI to avoid certain actors such as grenades.
 */
UCLASS()
class FREEDOMPHANTOMS_API UAvoidanceAction : public UUtilityAIAction
{
	GENERATED_BODY()
	
private:
	class ACombatAIController* CombatAIController;
	class ACombatCharacter* OwningCombatCharacter;


public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	virtual void Spawn(AAIController* Controller, APawn* Pawn) override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:
	void Flee();
};

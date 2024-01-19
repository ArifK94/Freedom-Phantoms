// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "RecruitAttackAction.generated.h"

/**
  * AI attack order from commander task.
 */
UCLASS()
class FREEDOMPHANTOMS_API URecruitAttackAction : public UUtilityAIAction
{
	GENERATED_BODY()

public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:

	/**
	* Move to an attack position which is similar to moving to defend position.
	*/
	void MoveToAttackPosition();

	/**
	* Move to high value target if ordered to target an enemy.
	*/
	void MoveToHVT();

	/**
	* No need to go up close to the HVT location, as long as the recruit NPC is within close distance & can see HVT, then this action does not need to run.
	*/
	bool CanSeeHVT() const;
};

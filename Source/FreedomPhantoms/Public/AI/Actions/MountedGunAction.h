// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "MountedGunAction.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMPHANTOMS_API UMountedGunAction : public UUtilityAIAction
{
	GENERATED_BODY()
	
private:
	EPathFollowingRequestResult::Type MoveToResult;

public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:
	void FindMountedGun();

	/**
	* Check if MG found is still valid to use.
	*/
	void MaintainMG();
};

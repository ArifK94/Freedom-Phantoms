// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "CombatAction.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMPHANTOMS_API UCombatAction : public UUtilityAIAction
{
	GENERATED_BODY()

private:
	mutable FTimerHandle THandler_Shoot;
	mutable FTimerHandle THandler_EndShooting;

public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	// NPC will need to have the ability to shoot 90% of the time.
	virtual bool CanRunAsynchronously(AAIController* Controller, APawn* Pawn) const override;

	virtual void Exit(AAIController* Controller, APawn* Pawn) override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:
	void CombatMode();

	void ShootAtEnemy();

	void EndShooting();

	void ThrowGrenade();

	bool CanThrowGrenade();

	void ReloadWeapon();

	void Aim();
};

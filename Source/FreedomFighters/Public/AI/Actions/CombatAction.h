// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UtilityAIAction.h"
#include "CombatAction.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMFIGHTERS_API UCombatAction : public UUtilityAIAction
{
	GENERATED_BODY()

private:
	class ACombatAIController* CombatAIController;
	class ACombatCharacter* OwningCombatCharacter;
	
public:
	virtual float Score(AAIController* Controller, APawn* Pawn) override;

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const override;

	// NPC will need to have the ability to shoot 90% of the time.
	virtual bool CanRunAsynchronously(AAIController* Controller, APawn* Pawn) const override;

	virtual void Spawn(AAIController* Controller, APawn* Pawn) override;

	virtual void Enter(AAIController* Controller, APawn* Pawn) override;

	virtual void Exit(AAIController* Controller, APawn* Pawn) override;

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) override;

private:
	void FaceTarget();

	void CombatMode();

	void ShootAtEnemy();

	void ThrowGrenade();

	bool CanThrowGrenade();

	void ReloadWeapon();
};

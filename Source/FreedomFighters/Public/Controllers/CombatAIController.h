// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CombatAIController.generated.h"

class ACombatCharacter;
class UAISenseConfig;
class UAISense;
class UAIPerceptionComponent;
class AWeapon;
UCLASS()
	class FREEDOMFIGHTERS_API ACombatAIController : public AAIController
{
	GENERATED_BODY()

private:
	ACombatCharacter* OwningCombatCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UBehaviorTree* BTAsset;

	UAIPerceptionComponent* PerceptionComp;
	AWeapon* CurrentWeapon;


	float CurrentDeltaTime;
	float BulletFireCountDown;
	float FiringWaitTime;
	bool IsCoolingDown;

public:
	ACombatAIController();

private:
	UAISenseConfig* GetPerceptionSenseConfig(TSubclassOf<UAISense> SenseClass);

	void SetVisionAngle();

	AActor* FindEnemy();

	void ShootAtEnemy();

	void StartFiring();

	void EndFiring();

	void ReloadWeapon();

private:
	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaTime) override;
};

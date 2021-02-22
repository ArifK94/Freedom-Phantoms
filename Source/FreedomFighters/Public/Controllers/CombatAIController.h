// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CombatAIController.generated.h"

class AGameModeManager;
class ACombatCharacter;
class ACommanderCharacter;
class UAISenseConfig;
class UAISense;
class UAIPerceptionComponent;
class AWeapon;
class USphereComponent;
UCLASS()
class FREEDOMFIGHTERS_API ACombatAIController : public AAIController
{
	GENERATED_BODY()

private:

	AGameModeManager* GameModeManager;
	ACombatCharacter* OwningCombatCharacter;
	USphereComponent* TargetSightSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UBehaviorTree* BTAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float AcceptanceRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float DistanceDiffSprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class UNavigationQueryFilter> FilterClass;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool StopOnOverlap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool UsePathfinding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool ProjectDestinationToNavigation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool CanStrafe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool AllowPartialPaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVector TargetDestination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TargetSightRadius;

	AActor* EnemyActor;

	UAIPerceptionComponent* PerceptionComp;
	AWeapon* CurrentWeapon;

	ACommanderCharacter* Commander;

	float CurrentDeltaTime;
	float BulletFireCountDown;
	float FiringWaitTime;
	bool IsCoolingDown;
	bool StayCombatAlert;
	bool HasChosenCover;
	bool CanFindCover;
	EPathFollowingRequestResult::Type CurrentMovement;

	FVector ChosenCoverPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<FVector> CoverLocationPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CoverRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int NumberOfCoverTraces;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		float MovementDebugSphereRadius;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		float MovementDebugLifetTime;

public:
	ACombatAIController();

private:
	void Init();

	void UpdateCharacterMovement();

	void UpdatCombatAlert();

	void UpdateSprint();

	UAISenseConfig* GetPerceptionSenseConfig(TSubclassOf<UAISense> SenseClass);

	void SetVisionAngle();

	AActor* FindEnemy();

	void ShootAtEnemy();

	void StartFiring();

	EPathFollowingRequestResult::Type MoveToTarget(float AcceptRadius);

	void CheckCommanderOrder();
	
	void GenerateCoverPoints(AActor* TargetActor);

	FVector GetClosestCoverPoint(AActor* TargetActor);

	void TakeCover();

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

private:
	virtual void Tick(float DeltaTime) override;
};

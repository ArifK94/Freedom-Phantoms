// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CombatAIController.generated.h"

class ACombatCharacter;
class ACommanderCharacter;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AActor* EnemyActor;

	UAIPerceptionComponent* PerceptionComp;
	AWeapon* CurrentWeapon;

	ACommanderCharacter* Commander;

	float CurrentDeltaTime;
	float BulletFireCountDown;
	float FiringWaitTime;
	bool IsCoolingDown;
	bool StayCombatAlert;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<FVector> CoverLocationPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVector ClosestCoverPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CoverRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Faction Manager", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AActor> WeaponClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Faction Manager", meta = (AllowPrivateAccess = "true"))
		AActor* WeaponObj;


public:
	ACombatAIController();

private:

	void Init();

	void UpdateCharacterMovement();

	void UpdatCombatAlert();

	UAISenseConfig* GetPerceptionSenseConfig(TSubclassOf<UAISense> SenseClass);

	void SetVisionAngle();

	AActor* FindEnemy();

	void ShootAtEnemy();

	void StartFiring();

	UFUNCTION(BlueprintCallable)
		EPathFollowingRequestResult::Type MoveToTarget();

	void CheckCommanderOrder();

	void FindCover(AActor* TargetActor);

	FVector GetClosestCoverPoint(AActor* TargetActor);

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

private:
	virtual void Tick(float DeltaTime) override;
};

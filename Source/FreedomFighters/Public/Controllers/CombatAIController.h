// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Characters/CommanderCharacter.h"
#include "CombatAIController.generated.h"

class AGameModeManager;
class ACombatCharacter;
class ACommanderCharacter;
class UAISenseConfig;
class UAISenseConfig_Sight;
class UAISense;
class UAIPerceptionComponent;
class AWeapon;
class APumpActionWeapon;
class USphereComponent;
class AStronghold;
class AMountedGun;
class UCommanderRecruit;
UCLASS()
class FREEDOMFIGHTERS_API ACombatAIController : public AAIController
{
	GENERATED_BODY()

private:

	AGameModeManager* GameModeManager;
	ACombatCharacter* OwningCombatCharacter;
	USphereComponent* TargetSightSphere;
	USphereComponent* MountedGunSphere;
	UAISenseConfig_Sight* AISightConfig;
	AMountedGun* MountedGun;
	AWeapon* CurrentWeapon;
	UAIPerceptionComponent* PerceptionComp;
	APumpActionWeapon* PumpActionWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AActor* EnemyActor;


	// to take defensive positions within the stronghold
	AStronghold* CurrentStronghold;
	class UCoverPointComponent* ChosenCoverPointComponent;
	ACommanderCharacter* Commander;


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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TimeBetweenShotsMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TimeBetweenShotsMax;

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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float ResetMovementCountdown;
	float CurrentResetMovementCountdown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float MountedGunSightRadius;


	FTimerHandle THandler_Sprint;
	FTimerHandle THandler_BeginFire;
	FTimerHandle THandler_EndFire;
	FTimerHandle THandler_FindEnemy;
	FTimerHandle THandler_CommanderOrders;
	FTimerHandle THandler_CombatAlert;
	FTimerHandle THandler_MountedGun;
	FTimerHandle THandler_FindCover;
	FTimerHandle THandler_BeginPeakCover;
	FTimerHandle THandler_EndPeakCover;
	FTimerHandle THandler_ResetMovement;


public:
	ACombatAIController();

	void FindEnemy();

private:
	void Init();

	UFUNCTION()
	void OnOrderReceived(UCommanderRecruit* RecruitInfo);
	bool HasAssignedOrderEvent;

	EPathFollowingRequestResult::Type MoveToTarget(float AcceptRadius, bool WalkNearTarget = true);

	UAISenseConfig* GetPerceptionSenseConfig(TSubclassOf<UAISense> SenseClass);

	void SetVisionAngle();

	void UpdateCharacterMovement();

	void UpdatCombatAlert();

	void BeginCoverPeak();
	void EndCoverPeak();

	void UpdateSprint();

	void ShootAtEnemy();

	void EndFiring();

	void FindMountedGun();

	void CheckCommanderOrder();

	void FindCover();
	
	void GenerateCoverPoints(AActor* TargetActor);

	FVector GetClosestCoverPoint(AActor* TargetActor);

	void TakeCover();

	/** If AI is stuck, then reset location appropiately */
	void ResetLocation();

	bool IsEnemyBehindMG(AActor* Enemy = nullptr);

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

private:
	virtual void Tick(float DeltaTime) override;




public:
	AActor* GetEnemyActor() {
		return EnemyActor;
	}

	void SetGuardingStronghold(AStronghold* Stronghold) {
		CurrentStronghold = Stronghold;
	}
};

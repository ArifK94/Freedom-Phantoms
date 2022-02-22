// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnumCollection.h"
#include "Characters/CommanderCharacter.h"
#include "CombatAIController.generated.h"

class ACombatCharacter;
class ACommanderCharacter;

class UAIMovementComponent;
class UPatrolFollowerComponent;
class UCoverFinderComponent;
class UTargetFinderComponent;
class UMountedGunFinderComponent;

class AWeapon;
class APumpActionWeapon;
class AStronghold;
class UCommanderRecruit;

UCLASS()
class FREEDOMFIGHTERS_API ACombatAIController : public AAIController
{
	GENERATED_BODY()

private:
	ACombatCharacter* OwningCombatCharacter;
	APumpActionWeapon* PumpActionWeapon;
	AActor* LastSeenEnemyActor;

	// to take defensive positions within the stronghold
	AStronghold* CurrentStronghold;
	class UCoverPointComponent* ChosenCoverPointComponent;
	ACommanderCharacter* Commander;


	bool StayCombatAlert;
	bool HasChosenCover;
	bool CanFindCover;

	FVector ChosenCoverPoint;
	FVector LastSeenPosition;

	FTargetSearchParameters* TargetSearchParams;

	float m_DelaTime;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UAIMovementComponent* AIMovementComponent;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UPatrolFollowerComponent* PatrolFollowerComponent;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCoverFinderComponent* CoverFinderComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTargetFinderComponent* TargetFinderComponent;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UMountedGunFinderComponent* MountedGunFinderComponent;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		CommanderOrders CurrentCommand;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AIBehaviourState CurrentBehaviourState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UBehaviorTree* BTAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float AcceptanceRadius;

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

	/** The sphere radius for recruits to move around a order position so that multiple recruits do not stick together in one place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float DestinationRadius;
	float DefaultDestinationRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AActor* EnemyActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool MoveToLastSeenEnemy;

	FTimerHandle THandler_ShootEnemy;
	FTimerHandle THandler_EndFire;
	FTimerHandle THandler_CommanderOrders;
	FTimerHandle THandler_MountedGun;
	FTimerHandle THandler_FindCover;
	FTimerHandle THandler_BeginPeakCover;
	FTimerHandle THandler_EndPeakCover;
	FTimerHandle THandler_LastSeenEnemy;
	FTimerHandle THandler_MoveToNearbyDestination;
	FTimerHandle THandler_PatrolStart;


	bool HasPlayedTargetFoundSound;

	// To prevent the NPC from constantly finding a new position to target destination
	bool HasChosenNearTargetDest;

public:
	ACombatAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:
	void Init();

	void ClearTimers();

	UFUNCTION()
		void OnHealthUpdate(FHealthParameters InHealthParameters);

	UFUNCTION()
		void OnMovementDestinationSet(AIBehaviourState BehaviourState);

	UFUNCTION()
		void OnMovementDestinationReached(FVector Destination);

	UFUNCTION()
		void OnRappelUpdated(ABaseCharacter* BaseCharacter);

	UFUNCTION()
		void OnOrderReceived(UCommanderRecruit* RecruitInfo);
	bool HasAssignedOrderEvent;

	UFUNCTION()
		void OnTargetSearchUpdate(FTargetSearchParameters TargetSearchParameters);

	EPathFollowingRequestResult::Type MoveToTarget(float AcceptRadius, bool WalkNearTarget = true);

	void UpdatCombatAlert();

	void BeginCoverPeak();
	void EndCoverPeak();

	void UpdateLastSeen();

	void ShootAtEnemy();

	void EndFiring();

	void ReloadWeapon();

	void FindMountedGun();

	void CheckCommanderOrder();

	void MoveToCover();

	void StartPatrol();

	void TargetFound();

	void MoveToRandomPoint();

	void MoveToNextPatrolPoint();

	bool IsNearCommander();

	bool IsNearCommander(FVector Location);

	void SetBehaviourState(AIBehaviourState State);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;


public:
	AActor* GetEnemyActor() {
		return EnemyActor;
	}

	void SetGuardingStronghold(AStronghold* Stronghold) {
		CurrentStronghold = Stronghold;
	}

	void SetTargetDestination(FVector Destination) {
		TargetDestination = Destination;
	}
};

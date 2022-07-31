// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnumCollection.h"
#include "Interfaces/Avoidable.h"
#include "Characters/CommanderCharacter.h"
#include "CombatAIController.generated.h"

class ACombatCharacter;
class ACommanderCharacter;

class UAIMovementComponent;
class UPatrolFollowerComponent;
class UCoverFinderComponent;
class UStrongholdDefenderComponent;
class UTargetFinderComponent;
class UMountedGunFinderComponent;

class AWeapon;
class UCommanderRecruit;

UCLASS()
class FREEDOMFIGHTERS_API ACombatAIController : public AAIController, public IAvoidable
{
	GENERATED_BODY()

private:
	float bDeltaTime;

	ACombatCharacter* OwningCombatCharacter;
	AActor* LastSeenEnemyActor;

	UCommanderRecruit* bRecruitInfo;
	ACommanderCharacter* Commander;

	/** Keep track of time in seconds spent on the enemy. Resets after getting another enemy. */
	float TimeOnCurrentEnemy;

	bool StayCombatAlert;

	// To prevent throwing a lot of grenades in short amount of time.
	bool HasThrownGrenade;

	FVector ChosenCoverPoint;
	bool CoverFound;


	// last location when enemy was seen.
	FVector LastSeenLocation;


	FTargetSearchParameters* TargetSearchParams;
	FAvoidableParams bAvoidableParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UUtilityAIComponent* UtilityAIComponent;
	class UCombatAction* CombatAction;

	UAIMovementComponent* AIMovementComponent;

	UPatrolFollowerComponent* PatrolFollowerComponent;

	UCoverFinderComponent* CoverFinderComponent;

	UStrongholdDefenderComponent* StrongholdDefenderComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTargetFinderComponent* TargetFinderComponent;

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

	/**
	* the range determines if the enemy is close by.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float EnemyCloseRange;

	/**
	* How much time spent on the same enemy?
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float TimeSpentOnEnemy;

	/**
	* Time Range when spending time on same enemy.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float TimeSpentOnEnemyRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool MoveToLastSeenEnemy;

	/** Min time when ready to throw greande when time on enemy meets criteria */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float GrenadeThrowTimeMin;

	/** Max time when ready to throw greande when time on enemy meets criteria */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float GrenadeThrowTimeMax;

	/**
	* Is Running for cover point?
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsRunningForCover;

	FTimerHandle THandler_CommanderOrders;
	FTimerHandle THandler_MoveToNearbyDestination;


	bool HasPlayedTargetFoundSound;

	// To prevent the NPC from constantly finding a new position to target destination
	bool HasChosenNearTargetDest;

public:
	ACombatAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	bool IsNearCommander();

	bool IsNearCommander(FVector Location);

	void SetBehaviourState(AIBehaviourState State);

	/**
	* AI SetFocalPoint custom method.
	*/
	void SetFocalPosition(FVector TargetLocation);

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

	UFUNCTION()
		void OnTargetSearchUpdate(FTargetSearchParameters TargetSearchParameters);

	virtual void OnNearbyActorFound_Implementation(FAvoidableParams AvoidableParams) override;


	void FindMountedGun();

	void UpdatCombatAlert();

	void CheckCommanderOrder();

	void TargetFound();

	void MoveToRandomPoint();


protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;


public:
	AIBehaviourState GetCurrentBehaviourState() { return CurrentBehaviourState; };

	AActor* GetEnemyActor() { return EnemyActor; }

	AActor* GetLastSeenEnemyActor() { return LastSeenEnemyActor; }

	void SetLastSeenEnemyActor(AActor* Actor) { LastSeenEnemyActor = Actor; }

	FTargetSearchParameters* GetTargetSearchParams() { return TargetSearchParams; }

	FAvoidableParams GetAvoidableParams() { return bAvoidableParams; }

	FVector GetTargetDestination() { return TargetDestination; }

	void SetTargetDestination(FVector Destination) { TargetDestination = Destination; }

	bool GetHasThrownGrenade() { return HasThrownGrenade; }

	void SetHasThrownGrenade(bool Value) { HasThrownGrenade = Value; }


	UAIMovementComponent* GetAIMovementComponent() { return AIMovementComponent; };

	UTargetFinderComponent* GetTargetFinderComponent() { return TargetFinderComponent; }

	UMountedGunFinderComponent* GetMountedGunFinderComponent() { return MountedGunFinderComponent; }

	UCoverFinderComponent* GetCoverFinderComponent() { return CoverFinderComponent; }

	UStrongholdDefenderComponent* GetStrongholdDefenderComponent() { return StrongholdDefenderComponent; }

	UPatrolFollowerComponent* GetPatrolFollowerComponent() { return PatrolFollowerComponent; }


	CommanderOrders GetCurrentCommand() { return CurrentCommand; };

	ACommanderCharacter* GetCommander() { return Commander; };

	float GetAcceptanceRadius() { return AcceptanceRadius; }

	UCommanderRecruit* GetRecruitInfo() { return bRecruitInfo; }

	float GetEnemyCloseRange() { return EnemyCloseRange; }

	float GetTimeSpentOnEnemy() { return TimeSpentOnEnemy; }

	float GetTimeSpentOnEnemyRange() { return TimeSpentOnEnemyRange; }

	bool GetCoverFound() { return CoverFound; }

	void SetCoverFound(bool Value) { CoverFound = Value; }

	bool GetIsRunningForCover() { return IsRunningForCover; }

	void SetIsRunningForCover(bool Value) { IsRunningForCover = Value; }

	bool GetMoveToLastSeenEnemy() { return MoveToLastSeenEnemy; }

	bool GetStayCombatAlert() { return StayCombatAlert; }

	void SetStayCombatAlert(bool Alert) { StayCombatAlert = Alert; }

	FVector GetLastSeenLocation() { return LastSeenLocation; }
};

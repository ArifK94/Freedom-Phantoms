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
class UCommanderRecruit;

UCLASS()
class FREEDOMFIGHTERS_API ACombatAIController : public AAIController
{
	GENERATED_BODY()

private:

	AGameModeManager* GameModeManager;
	ACombatCharacter* OwningCombatCharacter;
	USphereComponent* TargetSightSphere;
	UAISenseConfig_Sight* AISightConfig;
	AWeapon* CurrentWeapon;
	UAIPerceptionComponent* PerceptionComp;
	APumpActionWeapon* PumpActionWeapon;
	AActor* EnemyActor;
	AActor* LastSeenEnemyActor;
	CommanderOrders CurrentCommand;


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
	//EPathFollowingRequestResult::Type CurrentMovement;

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

	/** Duration in seconds to forget the last seen enemy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float LastSeenDuration;
	float LastSeenTimeCurrent;
	FVector LastSeenPosition;

	/** The sphere radius for recruits to move around a order position so that multiple recruits do not stick together in one place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		float DestinationRadius;

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
	FTimerHandle THandler_LastSeenEnemy;
	FTimerHandle THandler_MoveToNearbyDestination;


	bool HasPlayedTargetFoundSound;

	// To prevent the NPC from constantly finding a new position to target destination
	bool HasChosenNearTargetDest;

public:
	ACombatAIController();

	void FindEnemy();

private:
	void Init();

	void ClearTimers();

	UFUNCTION()
		void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo);

	UFUNCTION()
		void OnOrderReceived(UCommanderRecruit* RecruitInfo);
	bool HasAssignedOrderEvent;

	EPathFollowingRequestResult::Type MoveToTarget(float AcceptRadius, bool WalkNearTarget = true);

	UAISenseConfig* GetPerceptionSenseConfig(TSubclassOf<UAISense> SenseClass);

	void SetVisionAngle();

	void UpdatCombatAlert();

	void BeginCoverPeak();
	void EndCoverPeak();

	void UpdateLastSeen();

	void ShootAtEnemy();

	void EndFiring();

	void ReloadWeapon();

	void FindMountedGun();

	void CheckCommanderOrder();

	void FindCover();

	void TargetFound();

	void GenerateCoverPoints(AActor* TargetActor);

	FVector GetClosestCoverPoint(AActor* TargetActor);

	void TakeCover();

	void MoveToRandomPoint();

	FVector FindNearbyDestinationPoint();

	/** If AI is stuck, then reset location appropiately */
	void ResetLocation();

	bool IsEnemyBehindMG(AActor* Enemy = nullptr);

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	virtual void Tick(float DeltaTime) override;



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

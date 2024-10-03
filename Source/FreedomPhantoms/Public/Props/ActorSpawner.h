// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "ActorSpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorSpawnedSignature, AActorSpawner*, ActorSpawner, AActor*, Actor);

class UBoxComponent;
class AWeapon;
UCLASS()
class FREEDOMPHANTOMS_API AActorSpawner : public AActor
{
	GENERATED_BODY()

private:
	FTimerHandle THandler_Spawn;


	UPROPERTY()
		USceneComponent* Scene;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* SpawnArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* TriggerArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AActor> ActorClass;

	/** List of actor classes to spawn randomly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<AActor>> ActorClasses;

	/** Collection of the spawned actors. */
	UPROPERTY()
		TArray<AActor*> SpawnedActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName TargetPointTag;

	/** The faction which this actor belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TeamFaction FriendlyFaction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float SpawnRate;

	/** Delay before spawning an actor at the beginning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float SpawnFirstDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool TriggeredByEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool ConstantSpawning;

	/** Spawn actor navmesh? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool SpawnOnNav;

	/** Spawn actor without having to go to a target point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool FreeSpawn;

	/** Start spawning actors at the start of the game? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool SpawnAtStart;

	/** Limit the number of spawns if using Free Spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int FreeSpawnLimit;

	/** Prevent from triggering more than once. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool HasInitiatedSpawn;
	
	/** Set a specific weapon for the spawned characters, only if they are characters and not vehicles etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeapon> CharacterWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVector SpawnOffset;

public:	
	AActorSpawner();

	UPROPERTY(BlueprintAssignable)
		FOnActorSpawnedSignature OnActorSpawned;

	UFUNCTION(BlueprintCallable)
		void StartSpawnTimer();

	UFUNCTION(BlueprintCallable)
		void StopSpawnTimer();

protected:
	virtual void BeginPlay() override;

private:

	UFUNCTION()
		void OnHealthUpdate(FHealthParameters InHealthParameters);

	UFUNCTION()
		void OnSpawnActorDestroyed(AActor* DestroyedActor);

	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	

	void BeginSpawn();

	AActor* SpawnActor();

	FVector GetSpawnLocation(bool& IsValid);

	void SpawnWeapon(AActor* Actor);

	bool IsEnemyFaction(AActor* Actor);

	bool IsTargetPointValid(AActor* TargetPointActor);

	bool HasFreeTargetPoints();

	/** Has the spawn limit been reached? If using Free Spawn. */
	bool HasReachedSpawnLimit();

	TArray<AActor*> GetTargetPoints();

	void RemoveSpawnedActor(AActor* Actor);
};

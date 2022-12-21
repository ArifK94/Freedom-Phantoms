// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "ActorSpawner.generated.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName TargetPointTag;

	/** The faction which this actor belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TeamFaction FriendlyFaction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float SpawnRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool TriggeredByEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool ConstantSpawning;

	/** Prevent from triggering more than once. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool HasInitiatedSpawn;
	
	/** Set a specific weapon for the spawned characters, only if they are characters and not vehicles etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeapon> CharacterWeaponClass;

public:	
	AActorSpawner();

protected:
	virtual void BeginPlay() override;

private:

	UFUNCTION()
		void OnHealthUpdate(FHealthParameters InHealthParameters);

	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	void BeginSpawn();

	AActor* SpawnActor();

	void LoadSpawnLocation(FVector& Location, bool& IsValid);

	void SpawnWeapon(AActor* Actor);

	bool IsEnemyFaction(AActor* Actor);

	bool IsTargetPointValid(AActor* TargetPointActor);

	bool HasFreeTargetPoints();

	TArray<AActor*> GetTargetPoints();

	void StartSpawnTimer();

	void StopSpawnTimer();
};

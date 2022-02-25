// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "StructCollection.h"
#include "VehiclePathFollowerComponent.generated.h"


class UCurveFloat;
class AVehicleSplinePath;

class UCapsuleComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPathCompleteSignature, AVehicleBase*, Vehicle);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UVehiclePathFollowerComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	FTimerHandle THandler_Update;

	/** Used when vehicle movement type is set to Waiting */
	FTimerHandle THandler_WaitingMovment;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CollisionDetector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCurveFloat* CurveFloat;
	FTimeline CurveTimeline;

	/** The actor of the path to follow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AVehicleSplinePath* VehiclePath;

	/** The actor tag name of the path to follow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName VehiclePathTagName;

	/** The speed of completing the path: Low duration is fast. High duration is slow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float PathFollowDuration;

	/** How many laps until complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		uint8 TotalLaps;
	uint8 CurrentLap;

	/** Set the movement once is reaches the target, eg. hover over target location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EVehicleMovement TargetLocReachedVehicleMovementType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EVehicleMovement CurrentVehicleMovement;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Specified Target Destination", meta = (AllowPrivateAccess = "true"))
		FVector TargetDestination;

	/** If follow target location, then it needs to randomly spawn from target location, MIN coords */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Specified Target Destination", meta = (AllowPrivateAccess = "true"))
		FVector SpawnLocationMin;

	/** If follow target location, then it needs to randomly spawn from target location, MAX coords */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Specified Target Destination", meta = (AllowPrivateAccess = "true"))
		FVector SpawnLocationMax;

	/** The radius for random point on the navmesh when setting the target destination */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Specified Target Destination", meta = (AllowPrivateAccess = "true"))
		float RandomNavPointRadius;

	/** Move to a target location if set true, eg. used when spawning nearest checkpoint to player. This is set in the blueprints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Specified Target Destination", meta = (AllowPrivateAccess = "true"))
		bool FollowTargetDestination;

	/** Might want to destroy after completing the path */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool DestroyOnPathComplete;

public:
	UVehiclePathFollowerComponent();

	void ClearPath();

private:
	void Init();

	void Update();

	void FindPath();

	void ResumePath();

	UFUNCTION(BlueprintCallable)
		void SpawnRandomLocation();

	/** When target destination is given, the nearest navmesh should be found which allow AI to rappel for instance */
	UFUNCTION(BlueprintCallable)
		void FindNearestNav();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	//Event Handlers
public:
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void FollowSplinePath(float Value);

	/** Fly to random location, useful for transport aircrafts which would allow characters to rappel down */
	UFUNCTION()
		void MoveToLocation(float Value);

	UPROPERTY(BlueprintAssignable)
		FOnPathCompleteSignature OnPathComplete;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "StructCollection.h"
#include "VehiclePathFollowerComponent.generated.h"


class UCurveFloat;
class AVehicleSplinePath;
class ABaseCharacter;
class ARope;

class UCapsuleComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPathCompleteSignature, AVehicleBase*, Vehicle);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UVehiclePathFollowerComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	/** Used when resuming vehicle path */
	FTimerHandle THandler_ResumePath;

	/** Used when passengers are leaving the vehicle */
	FTimerHandle THandler_ExitPassenger;

	AVehicleBase* OwningVehicle;

	FVehicleSplinePoint CurrentSplinePoint;

	/** Stores spline points which have already been reached so that they are not processed again */
	TArray<int32> ProcessedPoints;

	FVector PreviousActorLocation;

	bool HasReachedPath;


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

	/** Follow path forever */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool HasInifiteLaps;

	/** Set the movement once is reaches the target, eg. hover over target location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EVehicleMovement TargetLocReachedVehicleMovementType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EVehicleMovement CurrentVehicleMovement;


	/** Smoothly move to first spline point rather than teleport to first spline point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool TransitionToSplineStart;

	/** Ignoring occupied vehicle paths when searching for vehicle paths. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IgnoreOccupiedPath;


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



	/** Vehicle Mesh sockets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passenger Exit", meta = (AllowPrivateAccess = "true"))
		FName LeftRopeSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passenger Exit", meta = (AllowPrivateAccess = "true"))
		FName RightRopeSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passenger Exit", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ARope> RopeClass;
	ARope* RopeLeft;
	ARope* RopeRight;



	/** Might want to destroy after completing the path */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool DestroyOnPathComplete;

public:
	UVehiclePathFollowerComponent();

	/** Stop following path */
	void Stop();

	void ClearPath();

	static void SetVehicleExit(ABaseCharacter* Character);

	static void SetRopeFree(FVehicletSeating VehicletSeat);

private:
	void Init();

	void FindPath();

	void StartPath(FString PathMethodName);

	void ResumePath();

	void ExitPassengers();

	void SpawnRope();

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

	UFUNCTION()
		void MoveToSplinePathStart(float Value);

	/** Fly to random location, useful for transport aircrafts which would allow characters to rappel down */
	UFUNCTION()
		void MoveToLocation(float Value);

	UPROPERTY(BlueprintAssignable)
		FOnPathCompleteSignature OnPathComplete;

	ARope* GetRopeLeft() { return RopeLeft; }
	ARope* GetRopeRight() { return RopeRight; }

};

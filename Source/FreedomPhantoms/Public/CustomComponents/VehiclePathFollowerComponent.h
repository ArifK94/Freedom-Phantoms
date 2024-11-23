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

class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehiclePointReachedSignature, FVehicleSplinePoint, VehicleSplinePoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPathCompleteSignature, AVehicleBase*, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPasengerExitSignature, FVehicletSeating, VehicletSeat);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UVehiclePathFollowerComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	/** Used when beginning follow path */
	UPROPERTY()
		FTimerHandle THandler_BeginPath;

	/** Used when resuming vehicle path */
	UPROPERTY()
		FTimerHandle THandler_ResumePath;

	/** Used when passengers are leaving the vehicle */
	UPROPERTY()
		FTimerHandle THandler_ExitPassenger;

	UPROPERTY()
		AVehicleBase* OwningVehicle;

	UPROPERTY()
		FVehicleSplinePoint CurrentSplinePoint;

	/** Stores spline points which have already been reached so that they are not processed again */
	UPROPERTY()
		TArray<int32> ProcessedPoints;

	UPROPERTY()
		FVector PreviousActorLocation;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USphereComponent* CollisionDetector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCurveFloat* CurveFloat;

	UPROPERTY()
		FTimeline CurveTimeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float CurrentDuration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsStopped;

	/** The actor of the path to follow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AVehicleSplinePath* VehiclePath;

	/** The next paths to take after finishing current path. List of paths are to be in order. Last connected path will be looped if set to loop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<AVehicleSplinePath*> ConnectedVehiclePaths;

	/** The actor tag name of the path to follow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName VehiclePathTagName;

	/** The speed of completing the path: Low duration is fast. High duration is slow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float PathFollowDuration;

	/** The original value sset for the duration */
	float DefaultPathFollowDuration;

	/** Add a delay before following path. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float BeginPathDelay;

	/** Starting point to follow the path. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float StartPointLength;

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

	/** Continuously find the vehicle path? Only works if IgnoreOccupiedPath is set to false. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool FindPathPerFrame;

	/** If vehicle path requires pilot character, then the vehicle will not follow path if no pilot exists or is alive in the vehicle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passenger Exit", meta = (AllowPrivateAccess = "true"))
		bool RequiresPilot;




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

	UPROPERTY()
		ARope* RopeLeft;

	UPROPERTY()
		ARope* RopeRight;

	/** Do all passengers exit the vehicle at the same time? eg. a car/ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passenger Exit", meta = (AllowPrivateAccess = "true"))
		bool SimultaneousExit;

	/** Might want to destroy after completing the path */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool DestroyOnPathComplete;

public:
	UVehiclePathFollowerComponent();

	/** Start following path */
	UFUNCTION(BlueprintCallable)
		void BeginPath();

	UFUNCTION(BlueprintCallable)
		void ResumePath();

	/** Stop following path */
	UFUNCTION(BlueprintCallable)
		void Stop();

	/** Go back to normal follow speed/ */
	UFUNCTION(BlueprintCallable)
		void ResumeNormalSpeed();

	/** Slow the vehicle down. */
	UFUNCTION(BlueprintCallable)
		void Slowdown();

	void ClearPath();

	static void SetVehicleExit(ABaseCharacter* Character);

	static void SetRopeFree(FVehicletSeating VehicletSeat);

private:
	void Init();

	void FindPath();

	void StartPath(FString PathMethodName);

	void ExitPassengers();

	void SpawnRope();

	void DetachRopes();

	bool CanFollowPath();

	UFUNCTION(BlueprintCallable)
		void SpawnRandomLocation();

	/** When target destination is given, the nearest navmesh should be found which allow AI to rappel for instance */
	UFUNCTION(BlueprintCallable)
		void FindNearestNav();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool ShouldStopVehicle();

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
		FOnVehiclePointReachedSignature OnVehiclePointReached;

	UPROPERTY(BlueprintAssignable)
		FOnPathCompleteSignature OnPathComplete;

	UPROPERTY(BlueprintAssignable)
		FOnPasengerExitSignature OnPasengerExit;

	EVehicleMovement GetCurrentVehicleMovement() { return CurrentVehicleMovement; }

	UCurveFloat* GetCurveFloat() { return CurveFloat; }


	ARope* GetRopeLeft() { return RopeLeft; }
	ARope* GetRopeRight() { return RopeRight; }

};

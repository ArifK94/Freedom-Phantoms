// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StructCollection.h"
#include "VehicleSplinePath.generated.h"

class USplineComponent;
UCLASS()
class FREEDOMPHANTOMS_API AVehicleSplinePath : public AActor
{
	GENERATED_BODY()
	

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USplineComponent* SplinePathComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleSplinePoint> VehicleSplinePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AActor* OccupiedVehicle;

	/** Override the duration of the VehiclePathComponent. Set to Specified to override the duration otherwise Normal will not override. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EVehicleSpeedType OverridePathDurationType;

	/** Set the new duration of the path. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float OverridePathDuration;

	/** Set a time limit for the vehicle to be on the path. Then destroy the vehicle. Set it to zero to not start timer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TimeLimit;

	UPROPERTY()
		FTimerHandle THandler_DurationLimit;

	/** Get random starting point for the vehicle to begin following this path? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool HasRandomStartingPoint;

	/** Can vehicles follow this path forever? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool HasInifiteLaps;

public:
	AVehicleSplinePath();

	static AVehicleSplinePath* FindVehiclePath(UWorld* World, FName TagName);

	int GetVehicleSplinePoint(FVector TargetLocation);
	FVehicleSplinePoint GetNextSplinePoint(int Index);

	void GetFirstSplinePoint(FVector& OutLocation, FRotator& OutRotation);

	/** Check if path is free to use by another actor. */
	bool IsPathFree();

	float GetOverridePathDuration(bool& IsOverride);

	void StartDurationLimit();

	/** Get the starting point of the spline length for vehicle tp start following from. */
	float GetStartPointLength();

private:
	void OnConstruction(const FTransform& Transform) override;


#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void UpdatePoints();

	void UpdatePointerPoints(int Index);

	void DestroyVehicle();


protected:
	virtual void BeginPlay() override;

public:

	USplineComponent* GetSplinePathComp() { return SplinePathComp; }

	TArray<FVehicleSplinePoint> GetVehicleSplinePoints() { return VehicleSplinePoints; }

	AActor* GetOccupiedVehicle() { return OccupiedVehicle; }

	void SetOccupantVehicle(AActor* Occupant) { OccupiedVehicle = Occupant; }

	bool GetHasInifiteLaps() { return HasInifiteLaps; }
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StructCollection.h"
#include "VehicleSplinePath.generated.h"

class USplineComponent;
UCLASS()
class FREEDOMFIGHTERS_API AVehicleSplinePath : public AActor
{
	GENERATED_BODY()
	

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USplineComponent* SplinePathComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleSplinePoint> VehicleSplinePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AActor* OccupiedVehicle;

public:
	AVehicleSplinePath();

	static AVehicleSplinePath* FindVehiclePath(UWorld* World, FName TagName);

	int GetVehicleSplinePoint(FVector TargetLocation);
	FVehicleSplinePoint GetNextSplinePoint(int Index);

	void GetFirstSplinePoint(FVector& OutLocation, FRotator& OutRotation);


private:
	void OnConstruction(const FTransform& Transform) override;


#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void UpdatePoints();

	void UpdatePointerPoints(int Index);

protected:
	virtual void BeginPlay() override;

public:

	USplineComponent* GetSplinePathComp() { return SplinePathComp; }

	TArray<FVehicleSplinePoint> GetVehicleSplinePoints() { return VehicleSplinePoints; }

	AActor* GetOccupiedVehicle() { return OccupiedVehicle; }

	void SetOccupantVehicle(AActor* Occupant) { OccupiedVehicle = Occupant; }

};

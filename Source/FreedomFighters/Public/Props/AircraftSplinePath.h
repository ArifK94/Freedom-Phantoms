// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StructCollection.h"
#include "AircraftSplinePath.generated.h"

class USplineComponent;
class UBoxComponent;



UCLASS()
class FREEDOMFIGHTERS_API AAircraftSplinePath : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline", meta = (AllowPrivateAccess = "true"))
		USplineComponent* SplinePathComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline", meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleSplinePoint> VehicleSplinePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline", meta = (AllowPrivateAccess = "true"))
		AActor* OccupiedVehicle;

public:	
	AAircraftSplinePath();

	FVehicleSplinePoint GetVehicleSplinePoint(FVector TargetLocation);
	FVehicleSplinePoint GetNextSplinePoint(int Index);

private:
	void OnConstruction(const FTransform& Transform) override;


#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void UpdatePointIndex();
	void UpdateCollisionBox();

protected:
	virtual void BeginPlay() override;

public:

	USplineComponent* GetSplinePathComp() {
		return SplinePathComp;
	}

	AActor* GetOccupiedVehicle() { return OccupiedVehicle; }

	void SetOccupantVehicle(AActor* Occupant) { OccupiedVehicle = Occupant; }
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AircraftSplinePath.generated.h"

class USplineComponent;
class UBoxComponent;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class AircraftSplineMovement : uint8
{
	Throttling	UMETA(DisplayName = "Throttling"),
	Hovering 	UMETA(DisplayName = "Hovering"),
	Stopping 	UMETA(DisplayName = "Stopping")
};

USTRUCT(BlueprintType)
struct FVehicleSplinePoint : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 PointIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector PointLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AircraftSplineMovement MovementType;
};

UCLASS()
class FREEDOMFIGHTERS_API AAircraftSplinePath : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline", meta = (AllowPrivateAccess = "true"))
		USplineComponent* SplinePathComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline", meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleSplinePoint> VehicleSplinePoints;

public:	
	AAircraftSplinePath();

	FVehicleSplinePoint GetVehicleSplinePoint(FVector TargetLocation);
	FVehicleSplinePoint GetNextSplinePoint(int Index);

private:
	void OnConstruction(const FTransform& Transform) override;

	void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;

	void UpdatePointIndex();
	void UpdateCollisionBox();

protected:
	virtual void BeginPlay() override;

public:

	USplineComponent* GetSplinePathComp() {
		return SplinePathComp;
	}
};

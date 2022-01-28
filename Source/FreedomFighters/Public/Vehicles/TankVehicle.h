// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vehicles/LandVehicle.h"
#include "StructCollection.h"
#include "TankVehicle.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMFIGHTERS_API ATankVehicle : public ALandVehicle
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FRotator RotationInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float PitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float PitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float YawMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float YawMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleWeapon> VehicleWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<AMountedGun*> WeaponsCollection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AMountedGun* CurrentWeapon;


public:
	ATankVehicle();

	UFUNCTION(BlueprintCallable)
		void SetRotationInput(FRotator Rotation);

private:
	void SpawnWeapons();

protected:
	virtual void BeginPlay() override;

};

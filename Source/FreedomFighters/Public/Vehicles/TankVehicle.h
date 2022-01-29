// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vehicles/LandVehicle.h"
#include "StructCollection.h"
#include "TankVehicle.generated.h"

class UTeamFactionComponent;
class UTargetFinderComponent;
class UShooterComponent;
UCLASS()
class FREEDOMFIGHTERS_API ATankVehicle : public ALandVehicle
{
	GENERATED_BODY()

private:
	int CurrentWeaponIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTeamFactionComponent* TeamFactionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTargetFinderComponent* TargetFinderComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UShooterComponent* ShooterComponent;


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

	/** The main cannon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVehicleWeapon VehicleWeaponMain;

	/** Turret weapons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleWeapon> VehicleWeaponTurrets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<AMountedGun*> WeaponsCollection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AMountedGun* CurrentWeapon;


public:
	ATankVehicle();

	UFUNCTION(BlueprintCallable)
		void SetRotationInput(FRotator InRotation);

	UFUNCTION(BlueprintCallable)
		void ChangeWeapon();

private:
	void SpawnVehicleWeapon(FVehicleWeapon VehicleWeapon);

protected:
	virtual void BeginPlay() override;

};

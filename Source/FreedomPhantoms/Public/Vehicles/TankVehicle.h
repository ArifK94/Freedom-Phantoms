// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vehicles/VehicleBase.h"
#include "StructCollection.h"
#include "TankVehicle.generated.h"

class USoundBase;
UCLASS()
class FREEDOMPHANTOMS_API ATankVehicle : public AVehicleBase
{
	GENERATED_BODY()

private:
	float m_DeltaTime;
	int CurrentWeaponIndex;

	UPROPERTY()
		AActor* TargetActor;

	FTimerHandle THandler_RandomChangeWeapon;

	UPROPERTY()
		FTargetSearchParameters TargetSearchParams;

	/** The audio to play when main turret is turning */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* TurretAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UTeamFactionComponent* TeamFactionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UTargetFinderComponent* TargetFinderComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UShooterComponent* ShooterComponent;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float PitchMin;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float DefaultPitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float PitchMax;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float DefaultPitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float YawMin;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float DefaultYawMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float YawMax;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float DefaultYawMax;

	/** Pitch clamped values can change depending on the Yaw value of the turret eg. turret will aim at the vehicle body when turning to 180 degrees meaning it will shoot through itself. This means the pitch min. value needs to change */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FClampChangePitch> ClampChangePitchValues;

	/** The main cannon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVehicleWeapon VehicleWeaponMain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AMountedGun* MainWeapon;

	/** Turret weapons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleWeapon> VehicleWeaponTurrets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<AMountedGun*> SecondaryWeapons;

	/** The speed of the turret when turning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TurretRotationFactor;

	/** How far off the turret rotation can determine to fire before reaching matching target rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TurretRotationErrorTolerance;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* TurretTurnSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* TurretTurnStopSound;

public:
	ATankVehicle();

	virtual void OnHealthUpdate(FHealthParameters InHealthParameters) override;

	UFUNCTION(BlueprintCallable)
		void ChangeSecondaryWeapon();

	UFUNCTION(BlueprintCallable)
		void RandomChangeWeapon();

	void SetCurrentWeapon(AMountedGun* InMountedGun, FVehicleWeapon InVehicleWeapon);

	//Event Handlers
public:
	UFUNCTION()
		void OnTargetSearchUpdate(FTargetSearchParameters TargetSearchParameters);

private:
	AMountedGun* SpawnVehicleWeapon(FVehicleWeapon VehicleWeapon);

	FRotator FaceTarget(AActor* Actor, FRotator& TargetRotation);

	FVehicleWeapon GetCurrentVehicleWeapon();

	void Shoot();

	void ChangePitchValue(float InYawValue);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual bool ShouldStopVehicle() override;

};

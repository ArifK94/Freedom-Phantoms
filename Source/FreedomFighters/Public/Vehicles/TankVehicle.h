// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vehicles/LandVehicle.h"
#include "StructCollection.h"
#include "TankVehicle.generated.h"

class UTeamFactionComponent;
class UTargetFinderComponent;
class UShooterComponent;
class USoundBase;
UCLASS()
class FREEDOMFIGHTERS_API ATankVehicle : public ALandVehicle
{
	GENERATED_BODY()

private:
	float m_DeltaTime;
	int CurrentWeaponIndex;
	AActor* TargetActor;

	FTimerHandle THandler_RandomChangeWeapon;

	/** The audio to play when main turret is turning */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* TurretAudio;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AMountedGun* CurrentWeapon;

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

	virtual void OnHealthUpdate(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo) override;

	UFUNCTION()
		void OnTargetSearchUpdate(AActor* Actor);

	UFUNCTION(BlueprintCallable)
		void SetRotationInput(FRotator InRotation);

	UFUNCTION(BlueprintCallable)
		void ChangeSecondaryWeapon();

	UFUNCTION(BlueprintCallable)
		void RandomChangeWeapon();

	void SetCurrentWeapon(AMountedGun* InMountedGun, FVehicleWeapon InVehicleWeapon);

private:
	AMountedGun* SpawnVehicleWeapon(FVehicleWeapon VehicleWeapon);

	FRotator FaceTarget(AActor* Actor, FRotator& TargetRotation);

	FVehicleWeapon GetCurrentVehicleWeapon();

	void Shoot();

	void ChangePitchValue(float InYawValue);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};

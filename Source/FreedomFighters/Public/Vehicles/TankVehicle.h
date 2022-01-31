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
	float m_DeltaTime;
	int CurrentWeaponIndex;
	AActor* TargetActor;

	FTimerHandle THandler_Shoot;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float PitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float YawMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float YawMax;

	/** The main cannon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVehicleWeapon VehicleWeaponMain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AMountedGun* MainWeapon;

	/** Turret weapons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleWeapon> VehicleWeaponTurrets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<AMountedGun*> WeaponsCollection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AMountedGun* CurrentWeapon;

	/** The speed of the turret when turning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TurretRotationFactor;

public:
	ATankVehicle();

	virtual void OnHealthUpdate(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo) override;

	UFUNCTION()
		void OnTargetSearchUpdate(AActor* Actor);

	UFUNCTION(BlueprintCallable)
		void SetRotationInput(FRotator InRotation);

	UFUNCTION(BlueprintCallable)
		void ChangeWeapon();

private:
	void SpawnVehicleWeapon(FVehicleWeapon VehicleWeapon);

	FRotator FaceTarget(AActor* Actor);

	void Shoot();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};

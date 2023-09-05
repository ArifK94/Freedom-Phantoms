// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/Weapon.h"
#include "StructCollection.h"
#include "AutoTurretWeapon.generated.h"

class UCameraComponent;
UCLASS()
class FREEDOMPHANTOMS_API AAutoTurretWeapon : public AActor
{
	GENERATED_BODY()

private:
	float m_DeltaTime;
	int CurrentWeaponIndex;

	UPROPERTY()
		AActor* TargetActor;

	FTimerHandle THandler_RandomChangeWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FollowCamera;

	/** The audio to play when main turret is turning */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* TurretAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UTeamFactionComponent* TeamFactionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UTargetFinderComponent* TargetFinderComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UShooterComponent* ShooterComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleWeapon> VehicleWeapons;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleWeapon> CurrentVehicleWeapons;

	UPROPERTY()
		FVehicleWeapon CurrentVehicleWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FRotator RotationInput;

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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		FName SurfaceImpactRowName;
	FSurfaceImpactSet* SurfaceImpactSet;

	/** Impulse applied to mesh when it explosed to boost up a little  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		float ExplosionImpulse;

	/** Explosion damage applied to nearby health components */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		float ExplosionDamage;

	/** Explosion sound attentuation  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* ExplosionAttenuation;

	/** The mesh to replace the original mesh once exploded  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		USkeletalMesh* ExplosionMesh;

	/** List containing all actor components to destroy when health reached zero eg. light components */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<UActorComponent*> DestroyableComponentList;


public:
	AAutoTurretWeapon();

	UFUNCTION(BlueprintCallable)
		void ChangeSecondaryWeapon();

	UFUNCTION(BlueprintCallable)
		void RandomChangeWeapon();

	bool UpdateCurrentWeapon(FVehicleWeapon InVehicleWeapon);

	/** Check if vehicle weapon is currently selected for use. */
	bool IsWeaponInUse(FVehicleWeapon InVehicleWeapon);

private:
	UFUNCTION()
		void OnHealthUpdate(FHealthParameters InHealthParameters);

	UFUNCTION()
		void OnTargetSearchUpdate(FTargetSearchParameters TargetSearchParameters);

	FRotator FaceTarget(AActor* Actor, FRotator& TargetRotation);

	void Shoot();

	void SpawnVehicleWeapons();


protected:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;

	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;
};

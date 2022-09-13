// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StructCollection.h"
#include "LandVehicle.generated.h"

class UCapsuleComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class USkeletalMesh;
class UHealthComponent;
class URadialForceComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class FREEDOMPHANTOMS_API ALandVehicle : public AActor
{
	GENERATED_BODY()

protected:
	FTimerHandle THandler_Update;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UHealthComponent* HealthComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		URadialForceComponent* RadialForceComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* EngineAudio;

	/** List containing all actor components to destroy when health reached zero eg. light components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<UActorComponent*> ActorComponentsToDestroy;

	/** Paramater name for the crossfade by param in the engine sound cue assigned to the engine audio component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName EngineSoundParamName;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicletSeating> VehicletSeating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleWeapon> VehicleWeapons;

	TArray<ABaseCharacter*> VehicleCharactersPtr;
	TArray<AMountedGun*> VehicleWeaponsPtr;


	/** This needs to be set on the tank's barrel socket for turret to face target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName CameraSocket;


	/** Impulse applied to mesh when it explosed to boost up a little  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float ExplosionImpulse;

	/** Explosion damage applied to nearby health components */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float ExplosionDamage;

	/** Particle to play when health reaches zero  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ExplosionEffect;
	
	/** Sound to play when health reaches zero  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* ExplosionSound;

	/** Explosion sound attentuation  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* ExplosionAttenuation;

	/** The mesh to replace the original mesh once exploded  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMesh* ExplosionMesh;

	/** Enabled physics during explosion  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool SimulateExplosionPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsDestroyed;

	int KillCount;
	
public:	
	ALandVehicle();

	UFUNCTION(BlueprintCallable)
		virtual void OnHealthUpdate(FHealthParameters InHealthParameters);

	UFUNCTION(BlueprintCallable)
		void AddComponentToDestroyList(UActorComponent* ActorComponent);

protected:
	virtual void BeginPlay() override;

	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	void Update();

	void SpawnVehicleWeapons();

	void SpawnVehicleSeatings();


private:	
	void ApplyExplosionDamage(FVector ImpactPoint, FHealthParameters InHealthParams);
	//void AddKill(UHealthComponent* DamagedActorHealth, UHealthComponent* OwnerHealth);
};

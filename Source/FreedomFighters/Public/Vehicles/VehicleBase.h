// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "VehicleBase.generated.h"

class UCapsuleComponent;
class UArrowComponent;
class UCameraComponent;
class USpringArmComponent;
class UHealthComponent;
class UVehiclePathFollowerComponent;
class URadialForceComponent;
class UUserWidget;
class AVehicleSplinePath;
class UPostProcessComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwitchSignature, AVehicleBase*, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleDestroySignature, AVehicleBase*, Vehicle);
UCLASS()
class FREEDOMFIGHTERS_API AVehicleBase : public AActor
{
	GENERATED_BODY()

protected:
	FTimerHandle THandler_Update;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UArrowComponent* EyePointComponent;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* EngineAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* PilotAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ThermalToggleAudio;

	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		URadialForceComponent* RadialForceComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UPostProcessComponent* ThermalVisionPPComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UVehiclePathFollowerComponent* VehiclePathFollowerComponent;



	/** This needs to be set on the tank's barrel socket for turret to face target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		FName CameraSocket;

	/** Paramater name for the crossfade by param in the engine sound cue assigned to the engine audio component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		FName EngineSoundParamName;

	/** Play at the start when taking control of the aircraft */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* InitialPilotSound;

	/** Interval based sound where pilot talks every few seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* RandomPilotSound;
	FTimerHandle THandler_RandomPiotSound;

	/** Kill Confirmed Sounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* KillConfirmedSound;

	/** Single Kill Parameter Name in sound base or cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		FName KillConfirmedParamName;

	/** Single Kill index defined in sound cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		int SingleKillIndex;

	/** Double Kill index defined in sound cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		int DoubleKillIndex;

	/** Multi Kill index defined in sound cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		int MultiKillIndex;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleWeapon> VehicleWeapons;
	TArray<FVehicleWeapon*> VehicleWeaponPtrList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int CurrentWeaponIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AMountedGun* CurrentWeapon;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicletSeating> VehicleSeats;
	TArray<FVehicletSeating*> VehicleSeatPtrList;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> HUDWidgetClass;
	UUserWidget* HUDWidget;
	

	/** Rotation speed for wheels, helicopter rotors etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float RotationSpeed;

	/** Current rotation speed for wheels, helicopter rotors etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CurrentRotationSpeed;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FRotator RotationInput;



	/** Use aircraft follow camera to navigate weapons, useful for ac130 to navigate all weapons to face in same direction when switching between them */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool UseFollowCamNavigation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool HighlightCharacters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target System", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ATargetSystemMarker> FriendlyMarkerClass;

	UPROPERTY()
		TArray<FTargetSystemNode> FriendlyMarkerNodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target System", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ATargetSystemMarker> EnemyMarkerClass;

	UPROPERTY()
		TArray<FTargetSystemNode> EnemySystemNodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target System", meta = (AllowPrivateAccess = "true"))
		TArray<UMaterialInterface*> ThermalMaterials;
	TArray<UMaterialInstanceDynamic*> ThermalMaterialInstances;
	int CurrentThermalMatIndex;

	/** Time taken for the SetViewTargetWithBlend function blend time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CameraSwitchDelay;
	FTimerHandle THandler_CameraSwitchDelay;



	/** Impulse applied to mesh when it explosed to boost up a little  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		float ExplosionImpulse;

	/** Explosion damage applied to nearby health components */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		float ExplosionDamage;

	/** Particle to play when health reaches zero  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ExplosionEffect;

	/** Sound to play when health reaches zero  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		USoundBase* ExplosionSound;

	/** Explosion sound attentuation  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* ExplosionAttenuation;

	/** The mesh to replace the original mesh once exploded  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		USkeletalMesh* ExplosionMesh;

	/** Enabled physics during explosion  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		bool SimulateExplosionPhysics;

	/** List containing all actor components to destroy when health reached zero eg. light components */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<UActorComponent*> DestroyableComponentList;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float WheelRPM;

public:	
	AVehicleBase();

	void RemovePassenger(int Index);

	void ChangeThermalVision();

	void SetPlayerControl(APlayerController* OurPlayerController, bool EnableThermalPP = true, bool ShowOutline = true);

	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);
	void AddControllerPitchInput(float Val, bool IsCameraRoam);
	void AddControllerYawInput(float Val, bool IsCameraRoam);
	void AddControllerPitchInput(float Val, FVehicletSeating VehicletSeating);
	void AddControllerYawInput(float Val, FVehicletSeating VehicletSeating);

	void BeginAim();
	void EndAim();

	void ChangeWeapon();

	UFUNCTION(BlueprintCallable)
		void SetRotationInput(FRotator InRotation);


private:
	void TimerTick();

	void UpdatePassengerSeats();

	void AddUIWidget();

	void RemoveUIWidget();

	void SpawnVehicleWeapons();

	void SpawnVehicleSeatings();

	/** Called when taking control of aircraft */
	void InitialContolSetup();

	void CreateThermalMatInstances();

	void UpdateCurrentThermalVision(float InWeight);

	void ShowOutlines(bool CanShow);

	void SetTargetSystem();

	void UpdateMarker(TArray<FTargetSystemNode> TargetSystemNode);

	/** Check if an actor has already been marked */
	bool DoesNodeExist(TArray<FTargetSystemNode> TargetSystemNodes, AActor* TargetActor);

	void UpdateWeaponView();

	/** Play aircraft crew random conversations at intervals */
	void PlayRandomPilotSound();

	void ApplyExplosionDamage(FVector ImpactPoint, FHealthParameters InHealthParams);

	UFUNCTION(BlueprintCallable)
		void AddComponentToDestroyList(UActorComponent* ActorComponent);

	void DestroyChildActor(TArray<AActor*> ParentActor);

protected:
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

//Event Handlers
public:
	UFUNCTION(BlueprintCallable)
		virtual void OnHealthUpdate(FHealthParameters InHealthParameters);

	UFUNCTION()
		void OnWeaponKillConfirm(FProjectileImpactParameters ProjectileImpactParameters);


	UPROPERTY(BlueprintAssignable)
		FOnWeaponSwitchSignature OnWeaponSwitch;

	UPROPERTY(BlueprintAssignable)
		FOnVehicleDestroySignature OnVehicleDestroy;



protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;


	USkeletalMeshComponent* GetMeshComponent() { return MeshComponent; }

	TArray<FVehicletSeating*> GetVehicleSeatPtrList() { return VehicleSeatPtrList; }

	TArray<FVehicletSeating> GetVehicleSeats() { return VehicleSeats; }

	AMountedGun* GetCurrentWeaponObj() { return CurrentWeapon; }

};

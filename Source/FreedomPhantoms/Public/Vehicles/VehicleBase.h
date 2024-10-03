// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "Interfaces/Targetable.h"
#include "VehicleBase.generated.h"

class UCapsuleComponent;
class UArrowComponent;
class UBoxComponent;
class UCameraComponent;
class USpringArmComponent;
class UHealthComponent;
class UOptimizerComponent;
class UVehiclePathFollowerComponent;
class URadialForceComponent;
class UUserWidget;
class AVehicleSplinePath;
class UPostProcessComponent;
class AProjectile;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwitchSignature, AVehicleBase*, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleDestroySignature, AVehicleBase*, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIncomingThreatUpdateSignature, FIncomingThreatParameters, IncomingThreatParams);
UCLASS()
class FREEDOMPHANTOMS_API AVehicleBase : public AActor, public ITargetable
{
	GENERATED_BODY()

protected:

	UPROPERTY()
		FTimerHandle THandler_Update;

	/** Save performance if vehicle does not require any tick or timer functions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool CanActorTick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* MeshComponent;

	/** If only the damaged mesh is imported as static mesh instead of skeletal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* DamagedMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UArrowComponent* EyePointComponent;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* EngineAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* PilotAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ThermalToggleAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* IncomingThreatAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* FrontCollider;

	/**
	* The front box component to damage actors when the vehicle is moving.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* FrontKillZoneComponent;
	
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UOptimizerComponent* OptimizerComponent;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int CurrentWeaponIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AMountedGun* CurrentWeapon;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicletSeating> VehicleSeats;


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

	/** The controller class that is being used to control the vehicle. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AController* UserController;

	/** Should the vehicle slowdown when an enemy is detected? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool SlowdownOnTargetFound;

	/** Should the vehicle stop when an enemy is detected? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool StopOnTargetFound;


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

	/** Due to some imported models having different forward rotation, the explosion mesh should be the same rotation as the normal vehicle mesh.
	So adding extra transform should solve this issue if cannot reimport mesh with same rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		FTransform ExplosionMeshTransformOffset;

	/** Should the actor be destroyed from the scene after destroying vehicle? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		bool DestroyOnDeath;

	/** Enabled physics during explosion  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		bool SimulateExplosionPhysics;

	/** Can the children in the normal mesh component be affected during visibility?  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (AllowPrivateAccess = "true"))
		bool CanPropagateMeshChildren;

	/** List containing all actor components to destroy when health reached zero eg. light components */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<UActorComponent*> DestroyableComponentList;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float WheelRPM;

	/** Type of actors to accept. Empty list will return all actor classes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<AActor>> CollisionClassFilters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool CheckFrontCollision;

	/** If stationary, then vehicle follow path component will not be needed for performance reasons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (AllowPrivateAccess = "true"))
		bool IsStationary;

	/** If it will not be controlled by player, then components like camera do not need to exist for performance reasons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (AllowPrivateAccess = "true"))
		bool HasNoPlayerInput;

	/** Should the tick event be running for the mesh component? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (AllowPrivateAccess = "true"))
		bool MeshComponentTickEnabled;

	bool ShowTargetSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (AllowPrivateAccess = "true"))
	TArray<AProjectile*> IncomingMissiles;

public:	
	AVehicleBase();

	void RemovePassenger(int Index);

	void ChangeThermalVision();

	void SetPlayerControl(APlayerController* OurPlayerController, bool EnableThermalPP = true, bool ShowOutline = true);
	void RemovePlayerControl();

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

	virtual void OnThreatInbound_Implementation(AProjectile* Missile) override;
	virtual void OnThreatOutbound_Implementation(AProjectile* Missile) override;

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

	/**
	* Has the front collider found any actors?
	*/
	bool IsFrontCollisionFound();

	void UpdateMarker(TArray<FTargetSystemNode> TargetSystemNode);

	/** Check if an actor has already been marked */
	bool DoesNodeExist(TArray<FTargetSystemNode> TargetSystemNodes, AActor* TargetActor);

	void UpdateWeaponView();

	/** Play aircraft crew random conversations at intervals */
	void PlayRandomPilotSound();

	void ApplyExplosionDamage(FVector ImpactPoint, FHealthParameters InHealthParams);

	void RemoveTargetSystem();

	UFUNCTION(BlueprintCallable)
		void SetShowDamaged(bool ShowDamaged);

	UFUNCTION(BlueprintCallable)
		void AddComponentToDestroyList(UActorComponent* ActorComponent);

	/** Destroy actor components which will not be needed to save performance */
	void OptimizeComponents();

	void DestroyChildActor(TArray<AActor*> ParentActor);

protected:
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	virtual bool ShouldStopVehicle();

//Event Handlers
public:

	UFUNCTION()
		void OnVehicleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
		virtual void OnHealthUpdate(FHealthParameters InHealthParameters);

	UFUNCTION()
		void OnWeaponKillConfirm(FProjectileImpactParameters ProjectileImpactParameters);


	UPROPERTY(BlueprintAssignable)
		FOnWeaponSwitchSignature OnWeaponSwitch;

	UPROPERTY(BlueprintAssignable)
		FOnVehicleDestroySignature OnVehicleDestroy;

	UPROPERTY(BlueprintAssignable)
		FOnIncomingThreatUpdateSignature OnIncomingThreatUpdate;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;


	USkeletalMeshComponent* GetMeshComponent() { return MeshComponent; }

	TArray<FVehicletSeating> GetVehicleSeats() { return VehicleSeats; }

	AMountedGun* GetCurrentWeaponObj() { return CurrentWeapon; }

	UVehiclePathFollowerComponent* GetVehiclePathFollowerComponent() { return VehiclePathFollowerComponent; }

};

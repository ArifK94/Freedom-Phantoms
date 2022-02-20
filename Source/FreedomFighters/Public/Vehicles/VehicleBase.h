// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Engine/DataTable.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "VehicleBase.generated.h"

class UCapsuleComponent;
class UArrowComponent;
class UCameraComponent;
class USpringArmComponent;
class UHealthComponent;
class URadialForceComponent;
class UUserWidget;
class AVehicleSplinePath;
class UCurveFloat;
class UPostProcessComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwitchSignature, AVehicleBase*, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPathCompleteSignature, AVehicleBase*, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestroySignature, AVehicleBase*, Vehicle);
UCLASS()
class FREEDOMFIGHTERS_API AVehicleBase : public AActor
{
	GENERATED_BODY()

protected:
	FTimerHandle THandler_Update;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CollisionDetector;

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


	/** Move to a target location if set true, eg. used when spawning nearest checkpoint to player. This is set in the blueprints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		bool FollowTargetLocation;

	/** If follow target location, then it needs to randomly spawn from target location, MIN coords */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		FVector SpawnLocationMin;

	/** If follow target location, then it needs to randomly spawn from target location, MAX coords */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		FVector SpawnLocationMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		FVector TargetDestination;

	/** The radius for random point on the navmesh when setting the target destination */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		float RandomNavPointRadius;

	/** Set the movement once is reaches the target, eg. hover over target location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		EVehicleMovement TargetLocReachedVehicleMovementType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		EVehicleMovement CurrentVehicleMovement;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicleWeapon> VehicleWeapons;
	TArray<FVehicleWeapon*> VehicleWeaponPtrList;
	int CurrentWeaponIndex;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FVehicletSeating> VehicleSeats;
	TArray<FVehicletSeating*> VehicleSeatPtrList;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> HUDWidgetClass;
	UUserWidget* HUDWidget;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path", meta = (AllowPrivateAccess = "true"))
		UCurveFloat* CurveFloat;
	FTimeline CurveTimeline;

	/** The actor of the path to follow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path", meta = (AllowPrivateAccess = "true"))
		AVehicleSplinePath* VehiclePath;

	/** The actor tag name of the path to follow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path", meta = (AllowPrivateAccess = "true"))
		FName VehiclePathTagName;

	/** The speed of completing the path: Low duration is fast. High duration is slow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path", meta = (AllowPrivateAccess = "true"))
		float PathFollowDuration;

	/** How many laps until complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path", meta = (AllowPrivateAccess = "true"))
		uint8 TotalLaps;
	uint8 CurrentLap;

	/** Rotation speed for wheels, helicopter rotors etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float RotationSpeed;

	/** Current rotation speed for wheels, helicopter rotors etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CurrentRotationSpeed;

	/** Might want to destroy after completing the path */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path", meta = (AllowPrivateAccess = "true"))
		float DestroyOnPathComplete;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FRotator RotationInput;



	/** Use aircraft follow camera to navigate weapons, useful for ac130 to navigate all weapons to face in same direction when switching between them */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool UseFollowCamNavigation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool HighlightCharacters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target System", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ATargetSystemMarker> FriendlyMarkerClass;
	TArray<FTargetSystemNode*> FriendlyMarkerNodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target System", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ATargetSystemMarker> EnemyMarkerClass;
	TArray<FTargetSystemNode*> EnemySystemNodes;

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



public:	
	AVehicleBase();

	void ChangeThermalVision();

	void SetPlayerControl(APlayerController* OurPlayerController, bool EnableThermalPP = true, bool ShowOutline = true);

	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);
	void AddControllerPitchInput(float Val, bool IsCameraRoam);
	void AddControllerYawInput(float Val, bool IsCameraRoam);
	void AddControllerPitchInput(float Val, FVehicletSeating* VehicletSeating);
	void AddControllerYawInput(float Val, FVehicletSeating* VehicletSeating);

	UFUNCTION(BlueprintCallable)
		void SetRotationInput(FRotator InRotation);

private:
	void Update();

	void AddUIWidget();

	void RemoveUIWidget();
	
	void FindPath();

	void SpawnVehicleWeapons();

	void SpawnVehicleSeatings();

	/** Called when taking control of aircraft */
	void InitialContolSetup();

	void UpdateCurrentThermalVision(float InWeight);

	void ShowOutlines(bool CanShow);

	void SetTargetSystem();

	void UpdateMarker(TArray<FTargetSystemNode*> TargetSystemNodes, TSubclassOf<ATargetSystemMarker> MarkerClass);

	bool DoesFriendlyNodeExists(AActor* TargetActor);

	bool DoesEnemyNodeExists(AActor* TargetActor);

	void UpdateWeaponView();

	/** Play aircraft crew random conversations at intervals */
	void PlayRandomPilotSound();

	void ApplyExplosionDamage(FVector ImpactPoint, FHealthParameters InHealthParams);

	UFUNCTION(BlueprintCallable)
		void SpawnRandomLocation();

	/** When target destination is given, the nearest navmesh should be found which allow AI to rappel for instance */
	UFUNCTION(BlueprintCallable)
		void FindNearestNav();

	UFUNCTION(BlueprintCallable)
		void AddComponentToDestroyList(UActorComponent* ActorComponent);

	void DestroyChildActor(TArray<AActor*> ParentActor);

protected:
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	//Event Handlers
public:
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void FollowSplinePath(float Value);

	/** Fly to random location, useful for transport aircrafts which would allow characters to rappel down */
	UFUNCTION()
		void MoveToLocation(float Value);

	UFUNCTION(BlueprintCallable)
		virtual void OnHealthUpdate(FHealthParameters InHealthParameters);

	UFUNCTION()
		void OnWeaponKillConfirm(FProjectileImpactParameters ProjectileImpactParameters);


	UPROPERTY(BlueprintAssignable)
		FOnWeaponSwitchSignature OnWeaponSwitch;

	UPROPERTY(BlueprintAssignable)
		FOnPathCompleteSignature OnPathComplete;

	UPROPERTY(BlueprintAssignable)
		FOnDestroySignature OnVehicleDestroy;





protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;
};

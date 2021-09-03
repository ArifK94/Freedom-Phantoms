#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Engine/DataTable.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "Aircraft.generated.h"

class AMountedGun;
class AAircraftSplinePath;
class UCurveFloat;
class UCapsuleComponent;
class UCameraComponent;
class UUserWidget;
class UPostProcessComponent;
class ABaseCharacter;
class ATargetSystemMarker;
class ARope;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponChangeSignature, AAircraft*, Aircraft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestorySignature, AAircraft*, Aircraft);

UCLASS()
class FREEDOMFIGHTERS_API AAircraft : public AActor
{
	GENERATED_BODY()


private:
#pragma region Ropes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName LeftRopeSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName RightRopeSocket;

	bool isLeftRappelOccupied;

	bool isRightRappelOccupied;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ARope> RopeClass;
	ARope* RopeLeft;
	ARope* RopeRight;

	FTimerHandle THandler_Rapelling;

	void WaitForRapelling();

	void UpdateOccupiedSeats();

public:
	void IsLeftRappelOccupied(bool value) {
		isLeftRappelOccupied = value;
	}

	void IsRightRappelOccupied(bool value) {
		isRightRappelOccupied = value;
	}

#pragma endregion

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* EngineAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* PilotAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ThermalToggleAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UPostProcessComponent* ThermalVisionPPComp;


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




	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<FAircraftWeapon> AircraftWeapons;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FAircraftWeapon CurrentAircraftWeapon;
	int CurrentWeaponIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> HUDWidgetClass;
	UUserWidget* HUDWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AAircraftSplinePath* AircraftPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName AircraftPathTagName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float PathFollowDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		uint8 TotalLaps;
	uint8 CurrentLap;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float WingRotationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CurrentWingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCurveFloat* CurveFloat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FRotator RotationInput;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AMountedGun* CurrentWeaponObj;

	/** Use aircraft follow camera to navigate weapons, useful for ac130 to navigate all weapons to face in same direction when switching between them */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool UseFollowCamNavigation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool HighlightCharacters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target System", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ATargetSystemMarker> FriendlyMarkerClass;
	TArray<FTargetSystemNode*> FriendlyMarkerNodes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target System", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ATargetSystemMarker> EnemyMarkerClass;
	TArray<FTargetSystemNode*> EnemySystemNodes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FLIR", meta = (AllowPrivateAccess = "true"))
		TArray<UMaterialInterface*> ThermalMaterials;
	TArray<UMaterialInstanceDynamic*> ThermalMaterialInstances;
	int CurrentThermalMatIndex;

	/** Time taken for the SetViewTargetWithBlend function blend time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CameraSwitchDelay;
	FTimerHandle THandler_CameraSwitchDelay;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FAircraftSeating> AircraftSeats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<FAircraftSeating> OccupiedSeats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		EAircraftMovement CurrentAircraftMovement;

	FTimeline CurveTimeline;
public:
	AAircraft();

	void SetPlayerControl(APlayerController* OurPlayerController, bool EnableThermalPP = true, bool ShowOutline = true);

	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);
	void AddControllerPitchInput(float Val, bool IsCameraRoam);
	void AddControllerYawInput(float Val, bool IsCameraRoam);
	void AddControllerPitchInput(float Val, FAircraftSeating AircraftSeating);
	void AddControllerYawInput(float Val, FAircraftSeating AircraftSeating);

	void BeginAim();
	void EndAim();

	void ChangeWeapon();

	UPROPERTY(BlueprintAssignable, Category = "Weapon UI")
		FOnWeaponChangeSignature OnUpdateWeaponUI;

	UPROPERTY(BlueprintAssignable)
		FOnDestorySignature OnAircraftDestroy;
	

	TArray<AMountedGun*> WeaponObjs; // holding this variable in the FAircraftWeapon returns null after weapon spawning

	void ChangeThermalVision();

private:

	void AddUIWidget();
	void RemoveUIWidget();

	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void FindPath();

	UFUNCTION()
		void FollowSplinePath(float Value);

	UFUNCTION()
		void OnWeaponKillConfirm(bool IsSingleKill, bool IsDoubleKill, bool IsMultiKill);

	void SpawnPassenger();

	void SpawnWeapon();

	void UpdateWeaponView();

	void ShowOutlines(bool CanShow);

	void SetTargetSystem();

	void UpdateMarker(TArray<FTargetSystemNode*> TargetSystemNodes, TSubclassOf<ATargetSystemMarker> MarkerClass);

	bool DoesFriendlyNodeExists(AActor* TargetActor);
	bool DoesEnemyNodeExists(AActor* TargetActor);

	void UpdateCurrentThermalVision(float InWeight);

	void CreateThermalMatInstances();

	/** Called when taking control of aircraft */
	void InitialContolSetup();

	void PlayRandomPilotSound();

	void OnDestroy();

	void DetroyChildActor(TArray<AActor*> ParentActor);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	USkeletalMeshComponent* GetMesh() {
		return Mesh;
	}


	FAircraftWeapon GetCurrentAircraftWeapon() {
		return CurrentAircraftWeapon;
	}

	AMountedGun* GetCurrentWeaponObj() {
		return CurrentWeaponObj;
	}

};

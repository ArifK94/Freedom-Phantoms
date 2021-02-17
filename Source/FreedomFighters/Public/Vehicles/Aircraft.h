#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/TimelineComponent.h"
#include "Engine/DataTable.h"

#include "Aircraft.generated.h"

class AWeapon;
class AAircraftSplinePath;
class UCurveFloat;
class UCapsuleComponent;
class UCameraComponent;
class UUserWidget;
class UPostProcessComponent;
class ABaseCharacter;
class ATargetSystemMarker;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponChangeSignature, AAircraft*, Aircraft);

UENUM(BlueprintType)
enum class AircraftMovement : uint8
{
	Grounded		UMETA(DisplayName = "Grounded"),
	Hovering		UMETA(DisplayName = "Hovering"),
	Stopping 		UMETA(DisplayName = "Stopping"),
	MovingForward	UMETA(DisplayName = "MovingForward"),
};

USTRUCT(BlueprintType)
struct FAircraftWeapon : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<AWeapon> Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName WeaponSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName CameraSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UUserWidget> HUD;
};



//USTRUCT(BlueprintType)
//struct FREEDOMFIGHTERS_API FThermalMaterial // don't want to create a new material everytime the post processing material is added or updated
//{
//	GENERATED_USTRUCT_BODY()
//
//public:
//	UPROPERTY()
//		UMaterialInstanceDynamic* MaterialInstance;
//
//	UPROPERTY()
//		UMaterialInterface* Material;
//
//	FThermalMaterial()
//	{
//	}
//};

USTRUCT(BlueprintType)
struct FREEDOMFIGHTERS_API FTargetSystemNode
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		ABaseCharacter* Character;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		ATargetSystemMarker* Marker; // the marker class containing the widget component

	FTargetSystemNode()
	{

	}
};




UCLASS()
class FREEDOMFIGHTERS_API AAircraft : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* EngineAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* PilotAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UPostProcessComponent* ThermalVisionPPComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AircraftMovement CurrentAircraftMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<FAircraftWeapon> AircraftWeapons;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FAircraftWeapon CurrentAircraftWeapon;
	int CurrentWeaponIndex;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AAircraftSplinePath* AircraftPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName AircraftPathTagName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float PathFollowDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		uint8 TotalLaps;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float WingRotationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CurrentWingSpeed;


	FTimeline CurveTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCurveFloat* CurveFloat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FRotator RotationInput;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AWeapon* CurrentWeaponObj;

		TArray<FTargetSystemNode*> TargetSystemNodes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target System", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ATargetSystemMarker> TargetMarkerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FLIR", meta = (AllowPrivateAccess = "true"))
		TArray<UMaterialInterface*> ThermalMaterials;
	TArray<UMaterialInstanceDynamic*> ThermalMaterialInstances;
	int CurrentThermalMatIndex;


public:
	AAircraft();

	void SetPlayerControl(APlayerController* OurPlayerController);

	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);

	void ChangeWeapon();

	UPROPERTY(BlueprintAssignable, Category = "Weapon UI")
		FOnWeaponChangeSignature OnUpdateWeaponUI;

	TArray<AWeapon*> WeaponObjs; // holding this variable in the FAircraftWeapon returns null after weapon spawning

	void ChangeThermalVision();

private:
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void FindPath();

	UFUNCTION()
		void FollowSplinePath(float Value);

	void SpawnWeapon();

	void UpdateWeaponView();

	void ShowOutlines();

	void SetTargetSystem();

	bool IfNodeExists(AActor* TargetActor);

	void UpdateCurrentThermalVision(float InWeight);

	void CreateThermalMatInstances();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;


	FAircraftWeapon GetCurrentAircraftWeapon() {
		return CurrentAircraftWeapon;
	}

	AWeapon* GetCurrentWeaponObj() {
		return CurrentWeaponObj;
	}

};

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
	AWeapon* WeaponObj;


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
UCLASS()
class FREEDOMFIGHTERS_API AAircraft : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet", meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* EngineAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FollowCamera;

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


private:
	FTimeline CurveTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCurveFloat* CurveFloat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FRotator RotationInput;

public:
	AAircraft();

	void SetPlayerControl(APlayerController* OurPlayerController);

	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);

	void ChangeWeapon();

	UPROPERTY(BlueprintAssignable, Category = "Weapon UI")
		FOnWeaponChangeSignature OnUpdateWeaponUI;

	TArray<AWeapon*> WeaponObjs;
	AWeapon* CurrentWeaponObj;


private:
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void FindPath();

	UFUNCTION()
		void FollowSplinePath(float Value);

	void SpawnWeapon();

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

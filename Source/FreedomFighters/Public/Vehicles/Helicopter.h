// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Helicopter.generated.h"

class ABaseCharacter;
class ARope;
class AAircraftSplinePath;
class UCurveFloat;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class HelicopterMovement : uint8
{
	Grounded		UMETA(DisplayName = "Grounded"),
	Hovering		UMETA(DisplayName = "Hovering"),
	Stopping 		UMETA(DisplayName = "Stopping"),
	MovingForward	UMETA(DisplayName = "MovingForward"),
};

UENUM(BlueprintType)
enum class HelicopterRole : uint8
{
	Pilot				UMETA(DisplayName = "Pilot"),
	SideGunner 			UMETA(DisplayName = "SideGunner"),
	MountedGunnner		UMETA(DisplayName = "MountedGunnner")
};

USTRUCT(BlueprintType)
struct FHelicopterSeating : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		HelicopterRole Role;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName SeatingSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 SeatPosition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 RappelAnimIndex;


	/** Check if rope is occupied */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool isRopeLeftSide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float CameraViewYawMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float CameraViewYawMax;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ABaseCharacter> Character;
	ABaseCharacter* CharacterObj;


	AHelicopter* OwningHelicopter;
};


UCLASS()
class FREEDOMFIGHTERS_API AHelicopter : public AActor
{
	GENERATED_BODY()

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* HelicopterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* RotorAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FHelicopterSeating> HelicopterSeating;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<FHelicopterSeating> OccupiedSeating;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Helicopter Actions", meta = (AllowPrivateAccess = "true"))
		HelicopterMovement CurrentMovement;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName LeftRopeSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName RightRopeSocket;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName AircraftPathTagName;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ARope> RopeClass;
	ARope* leftRopeObj;
	ARope* rightRopeObj;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AAircraftSplinePath* AircraftPath;

	bool isLeftRappelOccupied;

	bool isRightRappelOccupied;

	float CurrentDeltaTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float PathFollowDuration;


	FTimeline CurveTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCurveFloat* CurveFloat;
public:
	AHelicopter();

private:
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void FindPath();

	UFUNCTION()
		void FollowSplinePath(float Value);


	void SpawnPassenger();

	void WaitForRepelling();

	void UpdateOccupiedSeats();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void IsLeftRappelOccupied(bool value) {
		isLeftRappelOccupied = value;
	}

	void IsRightRappelOccupied(bool value) {
		isRightRappelOccupied = value;
	}
};

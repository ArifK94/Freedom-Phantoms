// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Helicopter.generated.h"

class ABaseCharacter;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
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
		float CameraViewYawMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float CameraViewYawMax;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Passengers", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ABaseCharacter> Character;
	ABaseCharacter* CharacterObj;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter Actions", meta = (AllowPrivateAccess = "true"))
		bool isMovingForward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter Actions", meta = (AllowPrivateAccess = "true"))
		bool isStopping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter Actions", meta = (AllowPrivateAccess = "true"))
		bool isHovering;

public:	
	AHelicopter();

private:
	void SpawnPassenger();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};

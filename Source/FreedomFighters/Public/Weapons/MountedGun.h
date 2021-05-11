// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "MountedGun.generated.h"

class UCameraComponent;
class ACharacter;
UCLASS()
class FREEDOMFIGHTERS_API AMountedGun : public AWeapon
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		FRotator RotationInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		float PitchMin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		float PitchMax;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		float YawMin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		float YawMax;

	/** Message to be displayed on the UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
		FName StopUsingMessage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		FName CameraPositionSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		FName CharacterPositionSocket;
	
	float DefaultFOV;
	float TargetFOV;

	FTimerHandle THandler_ZoomFOVIn;
	FTimerHandle THandler_ZoomFOVOut;

	// Help AI use Mounted Gun
	AActor* PotentialOwner;

public:
	AMountedGun();

	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);

	void SetPlayerControl(APlayerController* OurPlayerController, ACharacter* Character);
	void RemovePlayerControl(APlayerController* OurPlayerController, ACharacter* Character);

	virtual void SetIsAiming(bool isAiming) override;

	FVector GetCharacterStandPos();
	FRotator GetCharacterStandRot();


private:
	virtual void BeginPlay() override;

	void ZoomIn();
	void ZoomOut();

public:
	FName GetStopUsingMessage() {
		return StopUsingMessage;
	}

	AActor* GetPotentialOwner() {
		return PotentialOwner;
	}

	UCameraComponent* GetFollowCamera() {
		return FollowCamera;
	}

	void SetPotentialOwner(AActor* PotOwner) {
		PotentialOwner = PotOwner;
	}
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "MountedGun.generated.h"

class UCameraComponent;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
		FName CameraPositionSocket;
	

	float DefaultFOV;
	float TargetFOV;

	FTimerHandle THandler_ZoomFOVIn;
	FTimerHandle THandler_ZoomFOVOut;

public:
	AMountedGun();

	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);

	void SetPlayerControl(APlayerController* OurPlayerController);
	void RemovePlayerControl(APlayerController* OurPlayerController, class ACharacter* Character);

	FName GetStopUsingMessage() {
		return StopUsingMessage;
	}

	virtual void SetIsAiming(bool isAiming) override;

private:
	virtual void BeginPlay() override;


	void ZoomIn();
	void ZoomOut();

};

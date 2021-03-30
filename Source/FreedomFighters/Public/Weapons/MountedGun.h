// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "MountedGun.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMFIGHTERS_API AMountedGun : public AWeapon
{
	GENERATED_BODY()

private:
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
	
public:
	AMountedGun();

	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);


	FName GetStopUsingMessage() {
		return StopUsingMessage;
	}

};

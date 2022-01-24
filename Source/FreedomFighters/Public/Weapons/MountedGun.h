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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		float PitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		float PitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		float YawMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		float YawMax;

	/** Message to be displayed on the UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
		FName StopUsingMessage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		FName CameraPositionSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		FName CharacterPositionSocket;

	/** True sets character to stand behind MG, false does not set it as this can be used when character was spawned to use the mounted gun in a helicopter as a mounted gunner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		bool AdjustBehindMG;

	/** Use line trace to interact in order to use the weapon, aircraft built in MG's would likely to be set false */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		bool CanTraceInteraction;

	/** Can stop using MG, aircraft mounted guns may not have this set to true */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		bool CanExit;

	/** Make owner step back after dropping the MG, how much of a step back should they take? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mounted Gun", meta = (AllowPrivateAccess = "true"))
		float StepBackAmount;

	
	float DefaultFOV;
	float TargetFOV;

	FTimerHandle THandler_ZoomFOVIn;
	FTimerHandle THandler_ZoomFOVOut;

	// Help AI use Mounted Gun
	AActor* PotentialOwner;

public:
	AMountedGun();

	// Interactable interface methods
	virtual FString OnInteractionFound_Implementation(APawn* InPawn, AController* InController) override;
	virtual bool CanInteract_Implementation(APawn* InPawn, AController* InController) override;
	virtual AActor* OnPickup_Implementation(APawn* InPawn, AController* InController) override;

	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);

	void SetRotatioInput(FRotator Rotation);

	void SetPlayerControl(APlayerController* OurPlayerController, ACharacter* Character);
	void RemovePlayerControl(APlayerController* OurPlayerController, ACharacter* Character);

	virtual void DropWeapon(bool RemoveOwner = true, bool SimulatePhysics = false) override;

	void ResetCamera();

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

	FName GetCameraPositionSocket() {
		return CameraPositionSocket;
	}

	FName GetCharacterPositionSocket() {
		return CharacterPositionSocket;
	}

	AActor* GetPotentialOwner() {
		return PotentialOwner;
	}

	UCameraComponent* GetFollowCamera() {
		return FollowCamera;
	}

	FRotator GetRotationInput() {
		return RotationInput;
	}

	float GetPitchMin() {
		return PitchMin;
	}

	float GetPitchMax() {
		return PitchMax;
	}

	float GetYawMin() {
		return YawMin;
	}

	float GetYawMax() {
		return YawMax;
	}

	bool GetCanTraceInteraction() {
		return CanTraceInteraction;
	}

	bool GetAdjustBehindMG() {
		return AdjustBehindMG;
	}

	bool GetCanExitMG() {
		return CanExit;
	}

	void SetPotentialOwner(AActor* PotOwner) {
		PotentialOwner = PotOwner;
	}

	void SetAdjustBehindMG(bool Adjust) {
		AdjustBehindMG = Adjust;
	}

	void SetCanTraceInteraction(bool CanLineTrace) {
		CanTraceInteraction = CanLineTrace;
	}

	void SetCanExit(bool HasExit) {
		CanExit = HasExit;
	}

	void SetPitchMin(float Value) {
		PitchMin = Value;
	}

	void SetPitchMax(float Value) {
		PitchMax = Value;
	}

	void SetYawMin(float Value) {
		YawMin = Value;
	}
	void SetYawMax(float Value) {
		YawMax = Value;
	}
};

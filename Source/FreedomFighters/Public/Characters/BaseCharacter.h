// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "StructCollection.h"
#include "EnumCollection.h"
#include "BaseCharacter.generated.h"

class UHealthComponent;
class UAudioComponent;
class UPostProcessComponent;
class APlayerCameraManager;
class AAIController;
class UDataTable;

UCLASS(config = Game)
class FREEDOMFIGHTERS_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()


public:
	// Sets default values for this character's properties
	ABaseCharacter();

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;


#pragma region DataTables
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		UDataTable* VoiceClipsDatatable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		TArray<FName> VoiceSetRows;
	FVoiceClipSet* VoiceClipsSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		UDataTable* AccessoryDatatable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		FName AccessoryRowName;
	FAccessorySet* AccessorySet;

private:
	void RetrieveVoiceDataSet();
	void RetrieveAccessoryDataSet();

#pragma endregion

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName HeadSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName RightHandSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool UseRootMotion;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float CharacterSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float CharacterDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float ForwardInputValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float RightInputValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsCharacterInAir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool isSprinting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float aimYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float aimPitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float AimCameraFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", ClampMin = 0.1, ClampMax = 100))
		float AimCameraZoomSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isDead;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isTakingCover;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isCoveringHigh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isCoveringLow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isRepellingDown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isAtCoverCorner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isFacingCoverRHS;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool ReceeivedInitialDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool ChangedCharacterDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isAiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsInAircraft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int32 AircraftSeatPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FRotator CoverRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FVector PeakDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UPostProcessComponent* CharacterOutlinePPComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* VoiceAudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FAircraftSeating CurrentAircraftSeat;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UHealthComponent* HealthComp;

protected:

	float defaultMaxWalkSpeed;

	float CurrentDeltaTime;

	FVector DefaultCamSocketOffset;

	UAnimInstance* AnimInstance;

	bool canMoveForward;

	class ABaseCoverProp* CurrentCoverObj;

	bool CoverSelected;
	FVector WallLocation;
	FVector WallNormal;


private:

	float LastCharDirection;
	float LastForwardInputVal;
	float LastRightInput;

	float DefaultCameraFOV;

	float DefaultCamViewYawMin;
	float DefaultCamViewYawMax;

	FVector CoverStart;
	FVector CoverForwardAxis;


	FTimerHandle THandler_MovemntInputDisable;

	FTimerHandle THandler_ResetInitialDirectionBool;

	APlayerCameraManager* CamManager;


protected:

	void UpdateCameraView();


	void UpdateSpeed();


	void AimOffset();

	void UpdateCharacterMovement();

	UFUNCTION(BlueprintCallable, Category = "Health")
		virtual void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
		void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	void CheckCoverType();

	void UpdateDirection();

	void ResetInitialDirectionBool();

public:
	virtual void ShowCharacterOutline(bool CanShow);

	void BeginCrouch();

	void BeginSprint();
	void EndSprint();

	virtual void BeginAim();
	virtual void EndAim();

	void TakeCover();
	void CoverMovement(float Value);
	void MoveToCover();
	void EscapeCover();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


	virtual FVector GetPawnViewLocation() const override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;


public:
	FName GetHeadSocket() {
		return HeadSocket;
	}

	FVoiceClipSet* GetVoiceClipsSet() {
		return VoiceClipsSet;
	}

	FAccessorySet* GetAccessorySet() {
		return AccessorySet;
	}

	float GetCharacterSpeed() {
		return CharacterSpeed;
	}

	bool IsSprinting() {
		return isSprinting;
	}

	bool IsAiming() {
		return isAiming;
	}

	bool getisDead() {
		return isDead;
	}

	bool IsInHelicopter() {
		return IsInAircraft;
	}

	bool IsRepellingDown() {
		return isRepellingDown;
	}

	bool IsTakingCover() {
		return isTakingCover;
	}

	bool IsAtCoverCorner() {
		return isAtCoverCorner;
	}

	bool IsFacingCoverRHS() {
		return isFacingCoverRHS;
	}



	void IsTakingCover(bool Value) {
		isTakingCover = Value;
	}

	void IsFacingCoverRHS(bool Value) {
		isFacingCoverRHS = Value;
	}

	void IsAtCoverCorner(bool Value) {
		isAtCoverCorner = Value;
	}

	void SetRightInputValue(float Value) {
		RightInputValue = Value;
	}


	UPostProcessComponent* getCharacterOutlinePPComp() {
		return  CharacterOutlinePPComp;
	}

	UAudioComponent* getVoiceAudioComponent() {
		return VoiceAudioComponent;
	}

	void SetCharacterDirection(float Value) {
		CharacterDirection = Value;
	}

	void SetIsInAircraft(bool value) {
		IsInAircraft = value;
	}

	void SetIsRepellingDown(bool value) {
		isRepellingDown = value;
	}

	void SetAircraftSeatPosition(int32 position) {
		AircraftSeatPosition = position;
	}

	void SetAircraftSeat(FAircraftSeating CurrentSeating) {
		CurrentAircraftSeat = CurrentSeating;
	}

	FAircraftSeating GetAircraftSeat() {
		return CurrentAircraftSeat;
	}
};
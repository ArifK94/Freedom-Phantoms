// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Vehicles/Helicopter.h"

#include "BaseCharacter.generated.h"

class UHealthComponent;
class UAudioComponent;
class UPostProcessComponent;
class APlayerCameraManager;
class AAIController;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class CoverType : uint8
{
	Default			UMETA(DisplayName = "Default"),
	CornerLeft		UMETA(DisplayName = "CornerLeft"),
	CornerRight 	UMETA(DisplayName = "CornerRight")
};

USTRUCT(BlueprintType)
struct FCoverPoint : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FVector Location;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsOccupied;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		CoverType TypeOfCover;
};



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


protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
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
		bool isInHelicopter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int32 HelicopterSeatPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FRotator CoverRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FVector PeakDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FCoverPoint ChosenCoverPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Highlight", meta = (AllowPrivateAccess = "true"))
		UPostProcessComponent* CharacterOutlinePPComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* VoiceAudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FHelicopterSeating currentSeating;

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


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
		UHealthComponent* HealthComp;

	UFUNCTION(BlueprintCallable, Category = "Health")
		virtual void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);


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
	void MoveToCover();
	void EscapeCover();

protected:

	void MoveForward(float Value);

	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);


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
		return isInHelicopter;
	}

	bool IsRepellingDown() {
		return isRepellingDown;
	}

	bool IsTakingCover() {
		return isTakingCover;
	}


	void IsTakingCover(bool Value) {
		isTakingCover = Value;
	}

	void SetIsInHelicopter(bool value) {
		isInHelicopter = value;
	}

	void SetIsRepellingDown(bool value) {
		isRepellingDown = value;
	}

	void SetHelicopterSeatPosition(int32 position) {
		HelicopterSeatPosition = position;
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

	void SetHelicopterSeating(FHelicopterSeating CurrentSeating) {
		currentSeating = CurrentSeating;
	}

	FHelicopterSeating CurrentSeating() {
		return currentSeating;
	}
};
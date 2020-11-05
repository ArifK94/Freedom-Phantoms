// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Props/BaseCoverProp.h"


#include "BaseCharacter.generated.h"

class UHealthComponent;
class UAudioComponent;
class UPostProcessComponent;


UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class CoverPeakAction : uint8
{
	None		UMETA(DisplayName = "None"),
	Up			UMETA(DisplayName = "Up"),
	Down 		UMETA(DisplayName = "Down"),
	Left		UMETA(DisplayName = "Left"),
	Right		UMETA(DisplayName = "Right")
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isAtCoverCorner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isFacingLeftCover;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool ReceeivedInitialDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool ChangedCharacterDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isAiming;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FRotator CoverRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FVector PeakDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		CoverCornerType CurrentCoverType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		CoverPeakAction CurrentCoverPeakAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Highlight", meta = (AllowPrivateAccess = "true"))
		UPostProcessComponent* CharacterOutlinePPComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* VoiceAudioComponent;

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


	FTimerHandle THandler_MovemntInputDisable;

	FTimerHandle THandler_ResetInitialDirectionBool;


protected:

	void UpdateCameraView();


	void UpdateSpeed();

	UFUNCTION(BlueprintCallable, Category = "Character Actions")
		void BeginCrouch();

	void AimOffset();

	void UpdateCharacterMovement();


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
		UHealthComponent* HealthComp;

	UFUNCTION(BlueprintCallable, Category = "Health")
		void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);


private:
	void TakeCover();
	void EscapeCover();

	bool IsFacingCoverAngle();

	void UpdateDirection();

	void RenableMovementInput();

	void ResetInitialDirectionBool();


public:
	void ShowCharacterOutline(bool CanShow);


	UFUNCTION(BlueprintCallable, Category = "Character Actions")
		void BeginSprint();
	UFUNCTION(BlueprintCallable, Category = "Character Actions")
		void EndSprint();

	UFUNCTION(BlueprintCallable, Category = "Character Actions")
		virtual void BeginAim();
	UFUNCTION(BlueprintCallable, Category = "Character Actions")
		virtual void EndAim();


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
	bool getisDead() {
		return isDead;
	}

	UPostProcessComponent* getCharacterOutlinePPComp() {
		return  CharacterOutlinePPComp;
	}

	UAudioComponent* getVoiceAudioComponent()
	{
		return VoiceAudioComponent;
	}

	void SetCharacterDirection(float Value)
	{
		CharacterDirection = Value;
	}
};
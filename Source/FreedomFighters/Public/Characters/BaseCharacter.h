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
class AAircraft;
class AWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRappelUpdateignature, ABaseCharacter*, BaseCharacter);
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* AimCameraSpring;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		UDataTable* DeathAnimDatatable;
	FDeathAnimation* DeathAnimation;

private:
	void RetrieveVoiceDataSet();
	void RetrieveAccessoryDataSet();
	void RetrieveDeathAnimDataSet();

#pragma endregion

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* VoiceAudioComponent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName HeadSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName RightHandSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName ShoulderRightSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool UseRootMotion;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool UseAimCameraSpring;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float CharacterSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float CharacterDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float ForwardInputValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float RightInputValue;

	/** Delay time for destroying character after death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float DestroyDelayTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsCharacterInAir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool isSprinting;

	/** Toggling for sprint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsSprintDefault;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CoverDistance;

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
		FRotator CoverRotation;

	/** Left cover corner camera constraint pitch (x => minimum, y => maximum)  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVector2D CoverRotationLeftPitch;

	/** Left cover corner camera constraint yaw (x => minimum, y => maximum)  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVector2D CoverRotationLeftYaw;

	/** Right cover corner camera constraint pitch (x => minimum, y => maximum)  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVector2D CoverRotationRightPitch;

	/** Right cover corner camera constraint yaw (x => minimum, y => maximum)  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVector2D CoverRotationRightYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UPostProcessComponent* CharacterOutlinePPComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FAircraftSeating CurrentAircraftSeat;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAnimSequence* DeathAnimationAsset;

protected:

	float DefaultMaxWalkSpeed;

	float CurrentDeltaTime;

	FVector DefaultCamSocketOffset;

	UAnimInstance* AnimInstance;

private:
	FRotator RotationInput; // input for controller when looking around while taking cover and aiming

	float LastCharDirection;
	float LastForwardInputVal;
	float LastRightInput;

	float DefaultCameraFOV;

	FTimerHandle THandler_ResetInitialDirectionBool;
	FTimerHandle THandler_CharacterMovement;
	FTimerHandle THandler_CharacterDirection;

	// delay used to destroy character after death since we need to destory attached children but not all children such as weapons, these will be detached after some time
	FTimerHandle THandler_Destroyer;

	AAIController* DefaultAIController;

protected:

	virtual void InitTimeHandlers();

	virtual void ClearTimeHandlers();

	void UpdateCameraView();

	void UpdateSpeed();

	void AimOffset();

	void UpdateCharacterMovement();

	UFUNCTION(BlueprintCallable, Category = "Health")
		virtual void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo);
	virtual void PlayDeathAnim(AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo);

	UFUNCTION()
		void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void Landed(const FHitResult& Hit) override;

private:
	void UpdateDirection();

	void ResetInitialDirectionBool();

	void StartCover(FHitResult OutHit);

	void StartDestroy();
	void DetroyChildActor(TArray<AActor*> ParentActor);

public:
	FOnRappelUpdateignature OnRappelUpdate;

	virtual void ShowCharacterOutline(bool CanShow);

	void ToggleCrouch();

	void ToggleSprint();
	virtual void BeginSprint();
	virtual void EndSprint();

	virtual void BeginAim();
	virtual void EndAim();

	void TakeCover();
	void CoverMovement(float Value);
	virtual void StopCover();

	void SetAircraftSeat(FAircraftSeating Seating);
	virtual void SetIsRepellingDown(bool IsRappelling);

	void UpdateAimCamera();

	void PlayVoiceSound(USoundBase* Sound);

	void PostDeath();

protected:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


	virtual FVector GetPawnViewLocation() const override;

	virtual FRotator GetViewRotation() const override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;


public:
	AAIController* GetDefaultAIController() {
		return DefaultAIController;
	}


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

	bool GetIsInAircraft() {
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


	void SetUseAimCameraSpring(bool Value) {
		UseAimCameraSpring = Value;
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


	FRotator GetRotationInput() {
		return RotationInput;
	}

	UHealthComponent* GetHealthComp() {
		return HealthComp;
	}

	UPostProcessComponent* getCharacterOutlinePPComp() {
		return  CharacterOutlinePPComp;
	}

	void SetCharacterDirection(float Value) {
		CharacterDirection = Value;
	}


	FAircraftSeating GetAircraftSeat() {
		return CurrentAircraftSeat;
	}

};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "StructCollection.h"
#include "EnumCollection.h"
#include "BaseCharacter.generated.h"

class UHealthComponent;
class UOptimizerComponent;
class UAudioComponent;
class UPostProcessComponent;
class APlayerCameraManager;
class AAIController;
class UDataTable;
class AWeapon;
class AOrderIcon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterActionUpdateignature, FCharacterActionParameters, CharacterActionParameters);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRappelUpdateignature, ABaseCharacter*, BaseCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoverUpdateSignature, FCoverUpdateInfo, CoverUpdateInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMovementModeUpdateSignature, EMovementMode, PrevMovementMode, uint8, PreviousCustomMode);

UCLASS(config = Game)
class FREEDOMPHANTOMS_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()


public:
	// Sets default values for this character's properties
	ABaseCharacter();

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/*
	* Left shoulder spring arm.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* AimCameraLeftSpring;

	/*
	* Right shoulder spring arm.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* AimCameraRightSpring;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UOptimizerComponent* OptimizerComponent;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
	FName DeathAnimRowName;
	FDeathAnimation* DeathAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
	FName FootRowName;
	FSurfaceImpact* FootSurfaceImpact;

private:
	void RetrieveVoiceDataSet();
	void RetrieveAccessoryDataSet();
	void RetrieveDeathAnimDataSet();

#pragma endregion

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAudioComponent* VoiceAudioComponent;

protected:

	UPROPERTY()
	class AGameModeManager* GameModeManager;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
	FName HeadSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
	FName RightHandSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
	FName ShoulderLeftocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
	FName ShoulderRightSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
	FName LeftFootSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
	FName RightFootSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool UseRootMotion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool UseAimCameraSpring;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsFirstPersonView;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	bool isTakingCover;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	bool isCoveringHigh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	bool isCoveringLow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	bool CanCoverPeakUp;


	/**
	* To prevent the stand to crouch animations from playing if aleady in croching position, this is usefulf when changing from cover state back to default animation state.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool IsCurrentlyCrouched;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	float CoverDistance;

	/** Add an offset when checking if character can peak up, this can vary depending on character's cover animation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	FVector CoverPeakUpOffset;

	/**
	* Last position when taking cover. This is to allow the character to move back last position after moving out of corner cover using root motion.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	FVector LastCoverPosition;

	/**
	* Last rotation when taking cover. This is to allow the character to move back last position after moving out of corner cover using root motion.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	FRotator LastCoverRotation;

	/**
	* Add an offset for the character's Yaw axis when first taken cover.
	* This is to ensure the character is aligned correctly based on the cover obstacle.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	float CoverStartYawOffset;

	/**
	* Add an offset for the character's Yaw axis when moving during cover.
	* This is to ensure the character is aligned correctly based on the cover obstacle.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	float CoverMovementYawOffset;

	/**
	* Add an offset for the character's Yaw axis when moving during left cover corner.
	* This is to ensure the character is aligned correctly based on the cover obstacle.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	float CoverCornerLeftYawOffset;

	/**
	* Add an offset for the character's Yaw axis when moving during right cover corner.
	* This is to ensure the character is aligned correctly based on the cover obstacle.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	float CoverCornerRightYawOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	bool isAtCoverCorner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	bool isFacingCoverRHS;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool IsExitingVehicle;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool ChangedCharacterDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool isAiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsInVehicle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool isReviving;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	FRotator CoverRotation;

	/** Left cover corner camera constraint pitch (x => minimum, y => maximum)  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	FVector2D CoverRotationLeftPitch;

	/** Left cover corner camera constraint yaw (x => minimum, y => maximum)  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	FVector2D CoverRotationLeftYaw;

	/** Right cover corner camera constraint pitch (x => minimum, y => maximum)  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	FVector2D CoverRotationRightPitch;

	/** Right cover corner camera constraint yaw (x => minimum, y => maximum)  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	FVector2D CoverRotationRightYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPostProcessComponent* CharacterOutlinePPComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FVehicletSeating CurrentVehicleSeat;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class URappellerComponent* RappellerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AOrderIcon> OverheadIconClass;

	UPROPERTY()
	AOrderIcon* OverheadIcon;

protected:
	float DefaultMaxWalkSpeed;

	float CurrentDeltaTime;

	FVector DefaultCamSocketOffset;

private:
	FName DefaultCapsuleCollisionName;
	FName DefaultMeshCollisionName;

	UPROPERTY()
	AController* DefaultController;
	FVector DefaultMeshLocation;
	FRotator DefaultMeshRotation;
	FRotator RotationInput; // input for controller when looking around while taking cover and aiming

	float LastCharDirection;
	float LastForwardInputVal;
	float LastRightInput;

	float DefaultCameraFOV;

	float SprintSpeed;

	FTimerHandle THandler_DelayedBeginPlay;
	FTimerHandle THandler_CharacterMovement;
	FTimerHandle THandler_CharacterDirection;

	// delay used to destroy character after death since we need to destory attached children but not all children such as weapons, these will be detached after some time
	FTimerHandle THandler_Destroyer;

	UPROPERTY()
	AAIController* DefaultAIController;

protected:
	virtual void Init();

	virtual void InitTimeHandlers();

	virtual void ClearTimeHandlers();

	void UpdateCameraView();

	void UpdateSpeed();

	void AimOffset();

	/**
	* Is the cover crouch only will only allow character to crouch but not stand in cover.
	*/
	virtual void StartCover(FHitResult OutHit, bool IsCrouchOnly);

	virtual	void UpdateCharacterMovement();

	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void OnHealthUpdate(FHealthParameters InHealthParameters);

	UFUNCTION(BlueprintCallable, Category = "Rappelling")
	virtual void OnRappelChange(FRappellingParameters RappellingInfo);

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void HandleVoiceAudioFinished();

	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	virtual void Landed(const FHitResult& Hit) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation")
	virtual UAnimSequence* GetDeathAnim(FHealthParameters InHealthParameters);

private:
	void SpawnOverheadIcon();

	void UpdateDirection();

	void PlayFootstepSound(FHitResult HitInfo);

	USoundBase* GetFootstepSound(FHitResult HitInfo);

	void StartDestroy();
	void DetroyChildActor(TArray<AActor*> ParentActor);

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnRappelUpdateignature OnRappelUpdate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCharacterActionUpdateignature OnCharacterActionUpdate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCoverUpdateSignature OnCoverUpdate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnMovementModeUpdateSignature OnMovementModeUpdate;

	virtual FVector GetPawnViewLocation() const override;

	virtual FRotator GetViewRotation() const override;

	/** Default is idle, no interaction, the point is to reach default idle animation state.  */
	virtual void SetDefaultState();

	void SetFirstPersonView();

	// Ignoring death allows to call this function and display outline after character death.
	void ShowCharacterOutline(bool CanShow, bool IgnoreDeath = false);
	void ShowActorOutlineRecursive(TArray<AActor*> ParentActor, bool CanShow);
	void SetActorOutline(AActor* Actor, bool CanShow);

	virtual void UnCrouch(bool bClientSimulation = false) override;
	void ToggleCrouch();

	void ToggleSprint();
	virtual void BeginSprint();
	virtual void EndSprint();

	bool IsCharacterMoving();

	virtual void Jump() override;

	virtual void BeginAim();
	virtual void EndAim();

	void TakeCover();
	void CoverMovement(float Value);
	virtual void StopCover();
	void GetCorners(FVector WallNormal, bool& LineTraceLeft, bool& LineTraceRight);

	// rotate character to correctly face corners.
	void RotateToLeftCorner();
	void RotateToRightCorner();

	/**
	* Can character stand up while in cover?
	*/
	bool CanCoverStand();


	/**
	* Can character aim based on current state eg. while taking cover or crouching etc.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool CanAim();

	/**
	* Can character peak up while in cover?
	*/
	bool CanPerformCoverPeakUp();

	void SetVehicleSeat(FVehicletSeating Seat);
	virtual void SetIsExitingVehicle(bool IsExiting);

	void AttachIconToHead(AActor* Icon);

	void UpdateAimCamera();

	void PlayVoiceSound(USoundBase* Sound);

	/** Line trace from foot for footstep VFX */
	UFUNCTION(BlueprintCallable)
	void TraceFootstep();

	void PostDeath();

	virtual void DestroyUnusedComponents();

protected:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual void BeginPlay() override;

	void BeginDelayedPlay();

	virtual void Tick(float DeltaTime) override;

	virtual void Revived();

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

	bool GetIsInVehicle() {
		return IsInVehicle;
	}

	bool GetIsExitingVehicle() {
		return IsExitingVehicle;
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

	bool GetIsSprintDefault() {
		return IsSprintDefault;
	}

	void SetSprintDefault(bool Value) {
		IsSprintDefault = Value;
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

	void SetIsReviving(bool Value);


	void SetRightInputValue(float Value) {
		RightInputValue = Value;
	}

	void SetForwardInputValue(float Value);


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

	FVehicletSeating GetVehicletSeat() { return CurrentVehicleSeat; }

	AOrderIcon* GetOverheadIcon() { return OverheadIcon; }

	UAudioComponent* GetVoiceAudioComponent() { return VoiceAudioComponent; }

	URappellerComponent* GetRappellerComponent() { return RappellerComponent; }

};
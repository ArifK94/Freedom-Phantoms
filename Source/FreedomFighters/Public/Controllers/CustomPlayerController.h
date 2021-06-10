#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnumCollection.h"
#include "CustomPlayerController.generated.h"

class ABaseCharacter;
class ACommanderCharacter;
class ACombatCharacter;
class AAircraft;
class AWeapon;
class AInteractable;
class AMountedGun;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractiveFoundSignature, FName, ActionMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSupportPackageUpdateSignature, AInteractable*, SupportPackage, int32, ArrayPosition, bool, HasAddedItem);

UCLASS()
class FREEDOMFIGHTERS_API ACustomPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	ACombatCharacter* OwningCombatCharacter;

private:
	UPROPERTY(BlueprintAssignable)
		FOnInteractiveFoundSignature OnInteractionFound;

	UPROPERTY(BlueprintAssignable)
		FOnSupportPackageUpdateSignature OnSupportPackageUpdate;


	APawn* OwningPawn;
	ACommanderCharacter* OwningCommander;
	AMountedGun* MG;
	TeamFaction PlayerFaction;

	FTimerHandle THandler_CheckInteractable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AAircraft* ControlledAircraft;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float BaseLookUpRate;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> HealthWidgetClass;
	UUserWidget* HealthWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> WeaponAmmoWidgetClass;
	UUserWidget* WeaponAmmoWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> WeaponCrosshairhWidgetClass;
	UUserWidget* WeaponCrosshairWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> InventoryWidgetClass;
	UUserWidget* InventoryWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> SupportPackageWidgetClass;
	UUserWidget* SupportPackageWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> InteractWidgetClass;
	UUserWidget* InteractWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FText InteractKeyDisplayName;

	/** the interactable actor that is being line traced if found */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AInteractable* FocusedInteractable;

	AInteractable* CurrentInteractable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<AInteractable*> SupportPackages;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		int32 CurrentSupportPackageIndex;

	uint8 MaxSupportPackages;


	/** Line trace length for the interactable actor to be detected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float InteractionLength;
	
public:
	ACustomPlayerController();

	virtual void SetupInputComponent() override;

	/** This exists to help unarmed controller class not inherit all combat bind action mapping */
	virtual void InitInputComponent();

	virtual void OnPossess(APawn* InPawn) override;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void AddUIWidgets();

	UFUNCTION()
		void OnCharacterHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable)
		void OnAircraftDestroy(AAircraft* CurrentControlledAircraft);

	UFUNCTION()
		void OnCombatModeUpdated(ACombatCharacter* CombatCharacter);

	UFUNCTION()
		void OnRappelUpdated(ABaseCharacter* BaseCharacter);



	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	void MoveForward(float Value);
	void MoveRight(float Value);

	void BeginCrouch();

	void BeginSprint();
	void EndSprint();

	void BeginJump();
	void EndJump();

	virtual void BeginAim();
	void EndAim();

	void TakeCover();

	virtual void BeginFire();
	void EndFire();

	void SwitchWeapon();
	void ToggleThermalVision();

	void PickupInteractable();

	void BeginCheckInteractable();
	void CheckInteractable();

	void UseInteractableActor();

	void SortSupportPackages();

	void UseMountedGun();

	////////////// -------------------------- Aircraft Functions -------------------------- //////////////

	void ShowAircraftView();
	void HideAircraftView();


	////////////// -------------------------- Input Functions -------------------------- //////////////
	FTimerHandle THandler_InputHeld;
	float DesiredInputHoldTime;
	float CurrentInputHoldTime;

	void ClearInputHold();

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
		void PauseGame();


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> PauseMenuWidgetClass;
	UUserWidget* PauseMenuWidget;

	////////////// -------------------------- Commander Functions -------------------------- //////////////

private:
	void Recruit();

	void BeginAttackCommand();
	void EndAttackCommand();
	void Attack();

	void Defend();
	void BeginDefendCommand();
	void EndDefendCommand();

	void Follow();
	void BeginFollowCommand();
	void EndFollowCommand();
};

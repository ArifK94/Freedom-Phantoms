#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "CustomPlayerController.generated.h"

class UGameInstanceController;
class AGameStateBaseCustom;
class AGameStateBaseCustom;
class ABaseCharacter;
class ACommanderCharacter;
class ABaseObjective;
class UHealthComponent;
class AMapCamera;
class USphereComponent;
class AVehicleBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractiveFoundSignature, FString, ActionMessage, FString, KeyDisplay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSupportPackageUpdateSignature, ASupportPackage*, SupportPackage, int32, ArrayPosition, bool, HasAddedItem);

UCLASS()
class FREEDOMFIGHTERS_API ACustomPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	ACombatCharacter* OwningCombatCharacter;
	ACommanderCharacter* OwningCommander;

	FTimerHandle THandler_CheckInteractable;
private:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnInteractiveFoundSignature OnInteractionFound;

	UPROPERTY(BlueprintAssignable)
		FOnSupportPackageUpdateSignature OnSupportPackageUpdate;


	APawn* OwningPawn;
	UGameInstanceController* GameInstanceController;
	AGameStateBaseCustom* GameStateBaseCustom;
	TeamFaction PlayerFaction;

	USphereComponent* OverlapSphere;

	FTimerHandle THandler_PostDeath;

	/** The actor tag of the player start position on the map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName PlayerStartTagName;

	// Stores the existing viewport widgets, used to toggle visibility of widgets on pause
	TArray<UUserWidget*> OnViewWidgets;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AMapCamera* MapCamera;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AVehicleBase* ControlledVehicle;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> ObjectiveWidgetClass;
	UUserWidget* ObjectiveWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> EndGameWidgetClass;
	UUserWidget* EndGameWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> RadialMenuWidgetClass;
	UUserWidget* RadialMenuWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FString InteractKeyDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<ASupportPackage*> SupportPackages;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		ASupportPackage* CurrentSupportPackage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AActor* FocusedInteractableActor;

	// actor assigned on overlap begin & end 
	AActor* OverlappedInteractable;

	// Store the interactable after picking it up as player may used the interactable afterwards
	AActor* CollectedInteractableActor;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		int32 CurrentSupportPackageIndex;

	uint8 MaxSupportPackages;


	/** Line trace length for the interactable actor to be detected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float InteractionLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<ABaseObjective*> MissionObjectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		ABaseObjective* CurrentMissionObjective;

	/** To help overlap interactables such as weapons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float OverlapSpehereRadius;

	bool HasGameEnded;
	bool IsShowingRadialMenu;

private:
	void PostDeath();

	void DisplayEndGameUMG();

public:
	ACustomPlayerController();

	virtual void SetupInputComponent() override;

	/** This exists to help unarmed controller class not inherit all combat bind action mapping */
	virtual void InitInputComponent();

	virtual void OnPossess(APawn* InPawn) override;

	virtual void BeginPlay() override;

	// Rather than copying entire amount of code to be inherited by subclasses, common functionality should be passed here
	void InitBeginPlayCommon();

	/** This exists to help unarmed controller class not inherit all combat begin play functions */
	virtual void InitBeginPlayUncommon();


	virtual void Tick(float DeltaTime) override;



	void AddUIWidgets();


	UFUNCTION()
		void OnHealthUpdate(FHealthParameters InHealthParameters);

	UFUNCTION()
		void OnCharacterHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
		void OnVehicleDestroy(AVehicleBase* CurrentControlledVehicle);

	UFUNCTION()
		void OnCombatModeUpdated(ACombatCharacter* CombatCharacter);

	UFUNCTION()
		void OnRappelUpdated(ABaseCharacter* BaseCharacter);

	UFUNCTION()
		void OnObjectiveCompleted(ABaseObjective* Objective);

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
		void AddMissionObjective(ABaseObjective* Objective);

	void SpawnPlayer();

	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	void MoveForward(float Value);
	void MoveRight(float Value);

	void ZoomIn();
	void ZoomOut();

	void ToggleCrouch();

	void ToggleSprint();

	void BeginJump();
	void EndJump();

	virtual void BeginAim();
	void EndAim();

	void TakeCover();

	virtual void BeginFire();
	void EndFire();

	void BeginReload();

	void SwitchWeapon();

	UFUNCTION(BlueprintCallable)
		void OpenRadialMenu();

	UFUNCTION(BlueprintCallable)
		void CloseRadialMenu();
	void ToggleThermalVision();

	void PickupInteractable();

	void BeginCheckInteractable();
	void DetectInteractableByTrace();

	void UseInteractableActor();
	void DetectInteractable(AActor* Actor);
	AActor* DetectInteractableByOverlap();


	void SetControlledVehicle(AVehicleBase* InVehicle, bool IsContolled);
	void AddSupportPackage(ASupportPackage* InSupportPackage);
	void SortSupportPackages();

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UFUNCTION(BlueprintCallable)
		void SelectSupportPackage(int32 Index);

	bool CanAddSupportPackages();

	void UseMountedGun();
	void DropMountedGun();

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

public:
	void SetCurrentMissionObjective(ABaseObjective* Objective) {
		CurrentMissionObjective = Objective;
	}


};

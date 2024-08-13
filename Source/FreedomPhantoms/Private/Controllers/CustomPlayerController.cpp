#include "Controllers/CustomPlayerController.h"
#include "Managers/GameInstanceController.h"
#include "Managers/GameStateBaseCustom.h"
#include "Managers/GameHUDController.h"
#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"
#include "Weapons/Projectile.h"
#include "Weapons/AmmoCrate.h"
#include "Vehicles/VehicleBase.h"
#include "Props/CharacterSpawner.h"
#include "Props/SupportPackage.h"
#include "Props/MapCamera.h"
#include "Interfaces/Interactable.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "CustomComponents/VehiclePathFollowerComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetInputLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/InputSettings.h"



ACustomPlayerController::ACustomPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerStartTagName = "PlayerStart";

	DesiredInputHoldTime = .5f;

	InteractionLength = 350.0f;
	OverlapSpehereRadius = 100.0f;

	AutoReceiveInput = EAutoReceiveInput::Player0;

	CurrentSupportPackageIndex = -1;
	MaxSupportPackages = 4;

	HasGameEnded = false;
	ShouldRespawn = false;
	PlaySupportPackageSound = true;
}

void ACustomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	InitBeginPlayCommon();
}

void ACustomPlayerController::InitBeginPlayCommon()
{
	GameStateBaseCustom = Cast<AGameStateBaseCustom>(UGameplayStatics::GetGameState(GetWorld()));
	GameInstanceController = Cast<UGameInstanceController>(UGameplayStatics::GetGameInstance(GetWorld()));
	GameHUDController = Cast<AGameHUDController>(GetHUD());

	SpawnPlayer();


	if (!OwningCombatCharacter) {
		DisableInput(this);
		return;
	}

	OnCombatModeUpdated(OwningCombatCharacter);

	if (OwningCombatCharacter->GetMountedGun())
	{
		OwningCombatCharacter->GetMountedGun()->SetPlayerControl(this, OwningCombatCharacter);
	}
	else if (OwningCombatCharacter->GetVehicletSeat().OwningVehicle)
	{
		SetViewTargetWithBlend(OwningCombatCharacter->GetVehicletSeat().OwningVehicle, 0.0f);
	}


	// Character can be spawned in to use MG such as helicopter gunner
	UseMountedGun();


	PlayerFaction = OwningCombatCharacter->GetTeamFactionComponent()->GetSelectedFaction();

	InitBeginPlayUncommon();

	// Get Map camera
	AActor* MapActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMapCamera::StaticClass());
	MapCamera = Cast<AMapCamera>(MapActor);

	GameHUDController->AddPlayerWidgets();

	BeginCheckInteractable();

	OnInteractionFound.Broadcast("", "");
}

void ACustomPlayerController::InitBeginPlayUncommon()
{
	if (GameInstanceController)
	{
		SupportPackages.Empty();

		for (ASupportPackage* SP : GameInstanceController->GetSupportPackage())
		{
			SupportPackages.Add(SP);
		}
	}

	SortSupportPackages();

	if (SupportPackages.Num() > 0)
	{
		if (PlaySupportPackageSound)
		{
			CurrentSupportPackage->PlayVoiceOverSound(PlayerFaction);
			CurrentSupportPackage->PlayInteractSound();
		}

		CurrentSupportPackageIndex = SupportPackages.Num() - 1;
	}


	if (GameStateBaseCustom)
	{
		GameStateBaseCustom->OnGameEnded.AddDynamic(this, &ACustomPlayerController::OnGameEnded);
	}
}

void ACustomPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InitInputComponent();
}

void ACustomPlayerController::InitInputComponent()
{
	FInputActionBinding& PauseInput = InputComponent->BindAction("Pause", IE_Pressed, this, &ACustomPlayerController::PauseGame);
	PauseInput.bExecuteWhenPaused = true;

	InputComponent->BindAxis("Turn", this, &ACustomPlayerController::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ACustomPlayerController::TurnAtRate);

	InputComponent->BindAxis("LookUp", this, &ACustomPlayerController::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ACustomPlayerController::LookUpAtRate);

	FInputAxisBinding& MoveForwardInput = InputComponent->BindAxis("MoveForward", this, &ACustomPlayerController::MoveForward);
	MoveForwardInput.bExecuteWhenPaused = true;
	FInputAxisBinding& MoveRightInput = InputComponent->BindAxis("MoveRight", this, &ACustomPlayerController::MoveRight);
	MoveRightInput.bExecuteWhenPaused = true;

	FInputActionBinding& ZoomInInput = InputComponent->BindAction("ZoomIn", IE_Pressed, this, &ACustomPlayerController::ZoomIn);
	ZoomInInput.bExecuteWhenPaused = true;

	FInputActionBinding& ZoomOutInput = InputComponent->BindAction("ZoomOut", IE_Pressed, this, &ACustomPlayerController::ZoomOut);
	ZoomOutInput.bExecuteWhenPaused = true;


	InputComponent->BindAction("Jump", IE_Pressed, this, &ACustomPlayerController::BeginJump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACustomPlayerController::EndJump);

	InputComponent->BindAction("Aim", IE_Pressed, this, &ACustomPlayerController::BeginAim);
	InputComponent->BindAction("Aim", IE_Released, this, &ACustomPlayerController::EndAim);

	InputComponent->BindAction("Sprint", IE_Pressed, this, &ACustomPlayerController::ToggleSprint);

	InputComponent->BindAction("Crouch", IE_Pressed, this, &ACustomPlayerController::ToggleCrouch);

	InputComponent->BindAction("TakeCover", IE_Pressed, this, &ACustomPlayerController::TakeCover);

	InputComponent->BindAction("Fire", IE_Pressed, this, &ACustomPlayerController::BeginFire);
	InputComponent->BindAction("Fire", IE_Released, this, &ACustomPlayerController::EndFire);

	InputComponent->BindAction("Reload", IE_Pressed, this, &ACustomPlayerController::BeginReload);

	// number keys
	InputComponent->BindAction("SwitchWeapons", IE_Pressed, this, &ACustomPlayerController::SwitchWeapon);

	InputComponent->BindAction("InventoryToggle", IE_Pressed, this, &ACustomPlayerController::OpenRadialMenu);
	InputComponent->BindAction("InventoryToggle", IE_Released, this, &ACustomPlayerController::CloseRadialMenu);

	InputComponent->BindAction("ToggleNightVision", IE_Pressed, this, &ACustomPlayerController::ToggleThermalVision);

	InputComponent->BindAction("Pickup", IE_Pressed, this, &ACustomPlayerController::PickupInteractable);

	InputComponent->BindAction("UseInteractable", IE_Pressed, this, &ACustomPlayerController::UseInteractableActor);

	// Commander Input
	InputComponent->BindAction("Recruit", IE_Pressed, this, &ACustomPlayerController::Recruit);

	InputComponent->BindAction("Attack", IE_Pressed, this, &ACustomPlayerController::BeginAttackCommand);
	InputComponent->BindAction("Attack", IE_Released, this, &ACustomPlayerController::EndAttackCommand);

	InputComponent->BindAction("Defend", IE_Pressed, this, &ACustomPlayerController::BeginDefendCommand);
	InputComponent->BindAction("Defend", IE_Released, this, &ACustomPlayerController::EndDefendCommand);

	InputComponent->BindAction("Follow", IE_Pressed, this, &ACustomPlayerController::BeginFollowCommand);
	InputComponent->BindAction("Follow", IE_Released, this, &ACustomPlayerController::EndFollowCommand);
}


void ACustomPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	OwningPawn = InPawn;

	OwningCombatCharacter = Cast<ACombatCharacter>(OwningPawn);
	OwningCommander = Cast<ACommanderCharacter>(OwningPawn);

	// no point running this script if no player found
	if (OwningCombatCharacter == nullptr && OwningCommander == nullptr) {
		DisableInput(this);
		return;
	}

	if (OwningCombatCharacter)
	{
		// Grab the "Pickup Key" for it to be displayed on the UI 
		UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
		TArray <FInputActionKeyMapping> OutMappings;
		Settings->GetActionMappingByName("Pickup", OutMappings);
		if (OutMappings.Num() > 0)
		{
			InteractKeyDisplayName = UKismetInputLibrary::Key_GetDisplayName(OutMappings[0].Key).ToString();
		}
	}

	IsShowingRadialMenu = false;
}

void ACustomPlayerController::OnUnPossess()
{
	if (!UHealthComponent::IsActorAlive(OwningCombatCharacter))
	{
		return;
	}
	Super::OnUnPossess();
}

void ACustomPlayerController::SpawnPlayer()
{
	GetWorldTimerManager().ClearTimer(THandler_RespawnDelay);

	DisableInput(this);

	if (GameInstanceController == nullptr) {
		return;
	}

	FVector OutLocation;
	FRotator OutRotation;
	ACharacterSpawner::GetSpawnTransform(GetWorld(), OutLocation, OutRotation);

	OwningCombatCharacter = GameInstanceController->SpawnCombatCharacter(OutLocation, OutRotation);

	if (!OwningCombatCharacter) {
		return;
	}

	OwningCombatCharacter->SetPrimaryWeapon(GameInstanceController->SpawnPrimaryWeapon(OwningCombatCharacter));
	OwningCombatCharacter->SetSecondaryWeapon(GameInstanceController->SpawnSecondaryWeapon(OwningCombatCharacter));
	OwningCombatCharacter->AutoPossessAI = EAutoPossessAI::Disabled;
	OwningCombatCharacter->SetUseAimCameraSpring(true);

	// enable game controls, this will be needed after restarting a level 
	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(false);
	SetInputMode(InputMode);
	SetShowMouseCursor(false);

	// posses the player character
	Possess(OwningCombatCharacter);

	UHealthComponent* HealthComp = OwningCombatCharacter->GetHealthComp();
	HealthComp->SetRegenerateHealth(true);
	HealthComp->OnHealthChanged.AddDynamic(this, &ACustomPlayerController::OnHealthUpdate);

	OwningCombatCharacter->OnCombatUpdated.AddDynamic(this, &ACustomPlayerController::OnCombatModeUpdated);
	OwningCombatCharacter->OnRappelUpdate.AddDynamic(this, &ACustomPlayerController::OnRappelUpdated);

	if (OverlapSphere)
	{
		OverlapSphere->DestroyComponent();
	}

	OverlapSphere = NewObject<USphereComponent>(OwningCombatCharacter);

	if (OverlapSphere)
	{
		OverlapSphere->RegisterComponent();
		OverlapSphere->SetSphereRadius(OverlapSpehereRadius);
		OverlapSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ACustomPlayerController::OnOverlapBegin);
		OverlapSphere->AttachToComponent(OwningCombatCharacter->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	}


	EnableInput(this);
}

void ACustomPlayerController::PauseGame()
{
	if (HasGameEnded) {
		return;
	}

	UWorld* World = GetWorld();

	if (!World) {
		return;
	}

	// resume game
	if (UGameplayStatics::IsGamePaused(World))
	{
		if (MapCamera)
		{
			MapCamera->Deactivate();
		}

		if (PauseMenuWidget != nullptr && PauseMenuWidget->IsInViewport())
		{
			PauseMenuWidget->RemoveFromParent();
		}

		SetShowMouseCursor(false);

		for (int i = 0; i < OnViewWidgets.Num(); i++)
		{
			auto Widget = OnViewWidgets[i];

			if (Widget && Widget != GameHUDController->GetInventoryWidget())
			{
				Widget->SetVisibility(ESlateVisibility::Visible);
			}

		}

		// clear out the list, otherwise some widgets like aircraft widgets would be destroyed & this would cause an error 
		OnViewWidgets.Empty();

		UGameplayStatics::SetGamePaused(World, false);
	}
	else // pause game
	{
		if (MapCamera)
		{
			MapCamera->Activate();
		}

		// clear out the list, otherwise some widgets like aircraft widgets would be destroyed & this would cause an error 
		OnViewWidgets.Empty();

		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), OnViewWidgets, UUserWidget::StaticClass(), false);

		for (int i = 0; i < OnViewWidgets.Num(); i++)
		{
			OnViewWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (PauseMenuWidgetClass)
		{
			PauseMenuWidget = CreateWidget<UUserWidget>(World, PauseMenuWidgetClass);

			if (PauseMenuWidget)
			{
				PauseMenuWidget->AddToViewport();
			}
		}
		SetShowMouseCursor(true);

		UGameplayStatics::SetGamePaused(World, true);
	}
}

void ACustomPlayerController::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	if (HasGameEnded) {
		return;
	}

	if (!InHealthParameters.AffectedHealthComponent->IsAlive())
	{
		PreviousCombatCharacter = OwningCombatCharacter;
		PreviousOwningCommander = OwningCommander;

		OverlapSphere->DestroyComponent();
		OverlapSphere = nullptr;

		if (GameStateBaseCustom->GetCanPlayerRespawn())
		{
			GameHUDController->RemoveAllWidgets();

			// if not controlling a vehicle / support package, then respawn player.
			if (!ControlledVehicle)
			{
				GetWorldTimerManager().SetTimer(THandler_RespawnDelay, this, &ACustomPlayerController::RespawnPlayer, 1.f, false, 2.f);
			}
			else
			{
				ShouldRespawn = true;
			}
		}
		else
		{
			GameStateBaseCustom->EndGame(false);
			DisplayEndGameUMG();
		}
	}
}

void ACustomPlayerController::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) {
		return;
	}

	OverlappedInteractable = OtherActor;

	AWeapon* Weapon = Cast<AWeapon>(OverlappedInteractable);

	// if weapon has no owner
	// & not scavenged
	if (Weapon && Weapon->GetOwner() == nullptr && !Weapon->GetIsScavenged())
	{
		float Amount = Weapon->GetCurrentAmmo();
		// replenish ammo if same weapon based on weapon name
		if (OwningCombatCharacter->GetPrimaryWeapon() && OwningCombatCharacter->GetPrimaryWeapon()->GetWeaponName() == Weapon->GetWeaponName())
		{
			// if replenished then updated the scavenged flag
			if (OwningCombatCharacter->GetPrimaryWeapon()->ReplenishAmmo(Amount))
			{
				Weapon->SetIsScavenged(true);
				Weapon->Destroy(); // destroy the same weapon
			}
		} // for secondary weapon
		else if (OwningCombatCharacter->GetSecondaryWeaponObj() && OwningCombatCharacter->GetSecondaryWeaponObj()->GetWeaponName() == Weapon->GetWeaponName())
		{
			if (OwningCombatCharacter->GetSecondaryWeaponObj()->ReplenishAmmo(Amount))
			{
				Weapon->SetIsScavenged(true);
				Weapon->Destroy();
			}
		}
	}

}

void ACustomPlayerController::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// remove the UI interactable if the same actor
	if (OverlappedInteractable == OtherActor) {
		DetectInteractable(nullptr);
	}
}

void ACustomPlayerController::OnVehiclePointReached(FVehicleSplinePoint VehicleSplinePoint)
{
	if (VehicleSplinePoint.RemoveUserControl) {
		RemoveVehicleControl();
	}
}

void ACustomPlayerController::OnVehicleDestroy(AVehicleBase* CurrentControlledVehicle)
{
	if (ControlledVehicle != CurrentControlledVehicle) {
		return;
	}

	RemoveVehicleControl();

	if (ShouldRespawn)
	{
		RespawnPlayer();
		ShouldRespawn = false;
	}
}

void ACustomPlayerController::OnCombatModeUpdated(ACombatCharacter* CombatCharacter)
{
	// When using Mounted Gun in an aircraft, only MG camera can be shown and not the aircradt third person view
	if (CombatCharacter->IsInCombatMode() || OwningCombatCharacter->IsUsingMountedWeapon())
	{
		HideAircraftView();
	}
	else
	{
		ShowAircraftView();
	}
}

void ACustomPlayerController::OnRappelUpdated(ABaseCharacter* BaseCharacter)
{
	if (BaseCharacter->GetIsExitingVehicle())
	{
		OwningCombatCharacter->DisableInput(this);
		SetViewTargetWithBlend(OwningCombatCharacter, 0.2f);
	}
	else
	{
		OwningCombatCharacter->EnableInput(this);
	}
}

void ACustomPlayerController::OnGameEnded(bool hasMissionPassed)
{
	GameStateBaseCustom->EndGame(true);
	DisplayEndGameUMG();
}

void ACustomPlayerController::DisplayEndGameUMG()
{
	// In case character dies after mission success or failed
	if (HasGameEnded) {
		return;
	}

	HasGameEnded = true;
	EndFire();
	SetViewTargetWithBlend(OwningCombatCharacter);
	DisableInput(this);

	if (ControlledVehicle)
	{
		ControlledVehicle->Destroy();
	}

	UWidgetLayoutLibrary::RemoveAllWidgets(GetWorld());

	GameHUDController->AddEndGameWidget();
	SetShowMouseCursor(true);
	FInputModeUIOnly InData;
	InData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InData.SetWidgetToFocus(GameHUDController->GetEndGameWidget()->TakeWidget());
	SetInputMode(InData);
}

void ACustomPlayerController::RespawnPlayer()
{
	SpawnPlayer();

	PreviousCombatCharacter->CloneCharacter(OwningCombatCharacter);
	PreviousOwningCommander->ChangeCommander(OwningCommander);

	GameHUDController->AddPlayerWidgets();
}

void ACustomPlayerController::AddControllerPitchInput(float Val)
{
	if (Val == 0.f || !IsLocalPlayerController())
	{
		return;
	}

	if (ControlledVehicle) // if controlling an aircraft
	{
		ControlledVehicle->AddControllerPitchInput(Val);
	}
	// else if in an aircraft
	// mounted gun has its own control input so use that  in the next condition
	// allow player to rotate around character when repelling
	else if (OwningCombatCharacter && OwningCombatCharacter->GetVehicletSeat().OwningVehicle && !OwningCombatCharacter->IsUsingMountedWeapon() && !OwningCombatCharacter->GetIsExitingVehicle())
	{
		if (OwningCombatCharacter->IsInCombatMode())
		{
			OwningCombatCharacter->GetVehicletSeat().OwningVehicle->AddControllerPitchInput(Val, OwningCombatCharacter->GetVehicletSeat());
			OwningCombatCharacter->UpdateAimCamera();
		}
		else
		{
			OwningCombatCharacter->GetVehicletSeat().OwningVehicle->AddControllerPitchInput(Val, true); // Roam around aircraft view
		}
	}
	else if (OwningCombatCharacter && OwningCombatCharacter->IsUsingMountedWeapon())	// When using MG
	{
		OwningCombatCharacter->GetMountedGun()->AddControllerPitchInput(Val);
	}
	else if (OwningCombatCharacter && OwningCombatCharacter->IsTakingCover() && OwningCombatCharacter->IsAtCoverCorner())
	{
		OwningCombatCharacter->AddControllerPitchInput(Val);
	}
	else	// normal character movement
	{
		AddPitchInput(Val);
	}

}

void ACustomPlayerController::AddControllerYawInput(float Val)
{
	if (Val == 0.f || !IsLocalPlayerController())
	{
		return;
	}

	if (ControlledVehicle)
	{
		ControlledVehicle->AddControllerYawInput(Val);
	}
	else if (OwningCombatCharacter && OwningCombatCharacter->GetVehicletSeat().OwningVehicle && !OwningCombatCharacter->IsUsingMountedWeapon() && !OwningCombatCharacter->GetIsExitingVehicle())
	{
		if (OwningCombatCharacter->IsInCombatMode())
		{
			OwningCombatCharacter->GetVehicletSeat().OwningVehicle->AddControllerYawInput(Val, OwningCombatCharacter->GetVehicletSeat());
			OwningCombatCharacter->UpdateAimCamera();
		}
		else
		{
			OwningCombatCharacter->GetVehicletSeat().OwningVehicle->AddControllerYawInput(Val, true);
		}
	}
	else if (OwningCombatCharacter && OwningCombatCharacter->IsUsingMountedWeapon())
	{
		OwningCombatCharacter->GetMountedGun()->AddControllerYawInput(Val);
	}
	else if (OwningCombatCharacter && OwningCombatCharacter->IsTakingCover() && OwningCombatCharacter->IsAtCoverCorner())
	{
		OwningCombatCharacter->AddControllerYawInput(Val);
	}
	else
	{
		AddYawInput(Val);
	}

}

void ACustomPlayerController::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACustomPlayerController::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


void ACustomPlayerController::MoveForward(float Value)
{
	if (!OwningCombatCharacter) {
		return;
	}

	OwningCombatCharacter->SetForwardInputValue(Value);

	if (Value == 0.0f) {
		return;
	}

	if (MapCamera != nullptr && UGameplayStatics::IsGamePaused(GetWorld())) // navigate map
	{
		MapCamera->MoveForward(Value);
	}
	else // navigate character
	{
		if (!OwningCombatCharacter->InputEnabled()) {
			return;
		}

		if (OwningCombatCharacter->IsTakingCover())
		{
			if (Value < 0.0f)
			{
				OwningCombatCharacter->StopCover();
			}
		}
		else
		{
			// find out which way is forward
			const FRotator Rotation = GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			OwningCombatCharacter->AddMovementInput(Direction, Value);
		}
	}
}

void ACustomPlayerController::MoveRight(float Value)
{
	if (!OwningCombatCharacter) {
		return;
	}

	OwningCombatCharacter->SetRightInputValue(Value);

	if (Value == 0.0f) {
		return;
	}

	if (MapCamera != nullptr && UGameplayStatics::IsGamePaused(GetWorld())) // navigate map
	{
		MapCamera->MoveRight(Value);
	}
	else // navigate character
	{
		if (!OwningCombatCharacter->InputEnabled()) {
			return;
		}

		if (OwningCombatCharacter->IsTakingCover()) // move while in cover
		{
			if (OwningCombatCharacter->IsInCombatMode()) // break cover if in combat mode & moving
			{
				OwningCombatCharacter->StopCover();
			}
			else
			{
				OwningCombatCharacter->CoverMovement(Value);
			}
		}
		else // normal movement
		{
			//find out which way is right
			const FRotator Rotation = GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get right vector 
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			// add movement in that direction
			OwningCombatCharacter->AddMovementInput(Direction, Value);
		}
	}
}


void ACustomPlayerController::ZoomIn()
{
	if (MapCamera != nullptr && UGameplayStatics::IsGamePaused(GetWorld())) // zoom map
	{
		MapCamera->Zoom(-1.0f);
	}
}

void ACustomPlayerController::ZoomOut()
{
	if (MapCamera != nullptr && UGameplayStatics::IsGamePaused(GetWorld())) // zoom map
	{
		MapCamera->Zoom(1.0f);
	}
}

void ACustomPlayerController::ToggleCrouch()
{
	OwningCombatCharacter->ToggleCrouch();
}

void ACustomPlayerController::BeginJump()
{
	OwningCombatCharacter->Jump();
}

void ACustomPlayerController::EndJump()
{
	OwningCombatCharacter->StopJumping();
}

void ACustomPlayerController::ToggleSprint()
{
	OwningCombatCharacter->ToggleSprint();
}

void ACustomPlayerController::BeginAim()
{
	if (ControlledVehicle)
	{
		ControlledVehicle->BeginAim();
	}
	else
	{
		OwningCombatCharacter->BeginAim();
	}
}

void ACustomPlayerController::EndAim()
{
	if (ControlledVehicle)
	{
		ControlledVehicle->EndAim();
	}
	else
	{
		OwningCombatCharacter->EndAim();
	}
}

void ACustomPlayerController::TakeCover()
{
	if (OwningCombatCharacter->GetIsExitingVehicle()) {
		return;
	}

	OwningCombatCharacter->TakeCover();
}

void ACustomPlayerController::BeginFire()
{
	if (OwningCombatCharacter->GetIsExitingVehicle()) {
		return;
	}

	if (ControlledVehicle)
	{
		AWeapon* CurrentWeapon = ControlledVehicle->GetCurrentWeaponObj();
		if (CurrentWeapon) {
			CurrentWeapon->StartFire();
		}
	}
	else
	{
		OwningCombatCharacter->BeginFire();
	}
}

void ACustomPlayerController::EndFire()
{
	if (ControlledVehicle)
	{
		AWeapon* CurrentWeapon = ControlledVehicle->GetCurrentWeaponObj();
		if (CurrentWeapon) {
			CurrentWeapon->StopFire();
		}
	}
	else
	{
		OwningCombatCharacter->EndFire();
	}
}

void ACustomPlayerController::BeginReload()
{
	if (OwningCombatCharacter)
	{
		OwningCombatCharacter->BeginReload();
	}
}

void ACustomPlayerController::SwitchWeapon()
{
	if (OwningCombatCharacter->IsUsingMountedWeapon()) {
		return;
	}

	if (ControlledVehicle) {
		ControlledVehicle->ChangeWeapon();
	}
	else
	{
		OwningCombatCharacter->BeginWeaponSwap();
	}
}

void ACustomPlayerController::OpenRadialMenu()
{
	// if there is a radial widget created or already showing?
	if (!GameHUDController->GetInventoryWidget() || IsShowingRadialMenu) {
		return;
	}

	IsShowingRadialMenu = true;

	GetWorldTimerManager().ClearTimer(THandler_DelayedInput);

	DisableInput(this);
	SetShowMouseCursor(true);
	FInputModeGameAndUI InData;
	InData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InData.SetWidgetToFocus(GameHUDController->GetInventoryWidget()->TakeWidget());
	SetInputMode(InData);

	GameHUDController->GetInventoryWidget()->SetVisibility(ESlateVisibility::Visible);
}


void ACustomPlayerController::CloseRadialMenu()
{
	if (!IsShowingRadialMenu) {
		return;
	}

	IsShowingRadialMenu = false;

	// remove radial from screen
	if (GameHUDController->GetInventoryWidget()->IsInViewport()) {

		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		SetInputMode(InputMode);
		SetShowMouseCursor(false);

		GameHUDController->GetInventoryWidget()->SetVisibility(ESlateVisibility::Collapsed);

		GetWorldTimerManager().SetTimer(THandler_DelayedInput, this, &ACustomPlayerController::EnableInputDelay, 0.1f, true, .1f);
	}
}

void ACustomPlayerController::EnableInputDelay()
{
	EnableInput(this);
	GetWorldTimerManager().ClearTimer(THandler_DelayedInput);
}

void ACustomPlayerController::ToggleThermalVision()
{
	if (ControlledVehicle)
	{
		ControlledVehicle->ChangeThermalVision();
	}
}

void ACustomPlayerController::BeginCheckInteractable()
{
	GetWorldTimerManager().SetTimer(THandler_CheckInteractable, this, &ACustomPlayerController::DetectInteractableByTrace, 0.2f, true);
}

void ACustomPlayerController::DetectInteractable(AActor* Actor)
{
	FocusedInteractableActor = nullptr;
	FString ActionMessage = "";
	FString KeyInputDisplayName = InteractKeyDisplayName;

	if (Actor && Actor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		auto CanInteract = IInteractable::Execute_CanInteract(Actor, OwningCombatCharacter, this);

		if (CanInteract)
		{
			FocusedInteractableActor = Actor;
			KeyInputDisplayName = IInteractable::Execute_GetKeyDisplayName(Actor);
			ActionMessage = IInteractable::Execute_OnInteractionFound(Actor, OwningCombatCharacter, this);
		}
	}

	// assign default interaction key if none assigned from the interface methods
	if (UKismetStringLibrary::IsEmpty(KeyInputDisplayName)) {
		KeyInputDisplayName = InteractKeyDisplayName;
	}

	OnInteractionFound.Broadcast(ActionMessage, KeyInputDisplayName);
}

void ACustomPlayerController::DetectInteractableByTrace()
{
	if (OwningCombatCharacter == nullptr) {
		return;
	}

	AActor* HitActor = nullptr;
	FHitResult OutHit;
	FVector Start = OwningCombatCharacter->FollowCamera->GetComponentLocation();

	FVector ForwardVector = OwningCombatCharacter->FollowCamera->GetForwardVector();
	FVector End = ((ForwardVector * InteractionLength) + Start);


	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwningCombatCharacter);
	QueryParams.bTraceComplex = false;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;

	if (GetWorld()->LineTraceSingleByObjectType(OutHit, Start, End, ObjectParams, QueryParams))
	{
		if (OutHit.bBlockingHit)
		{
			HitActor = OutHit.GetActor();
		}
	}

	// if no line trace hit then check for overlapped actors
	if (HitActor == nullptr)
	{
		HitActor = DetectInteractableByOverlap();
	}

	DetectInteractable(HitActor);
}

AActor* ACustomPlayerController::DetectInteractableByOverlap()
{
	if (!OverlapSphere) {
		return nullptr;
	}

	AActor* ChosenActor = nullptr;
	TArray<AActor*> OverlappedActors;
	float TargetSightDistance = OverlapSpehereRadius;

	OverlapSphere->GetOverlappingActors(OverlappedActors, UInteractable::StaticClass());

	for (int i = 0; i < OverlappedActors.Num(); i++)
	{
		AActor* OverlappedActor = OverlappedActors[i];

		if (OverlappedActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
		{
			ChosenActor = OverlappedActor;
		}
	}

	return ChosenActor;
}

void ACustomPlayerController::PickupInteractable()
{
	if (!FocusedInteractableActor) {
		return;
	}

	CollectedInteractableActor = FocusedInteractableActor;

	if (CollectedInteractableActor)
	{
		// Setting the owner will help the pickup menthods to call the owning character and it's components
		CollectedInteractableActor->SetOwner(OwningCombatCharacter);
		OverlappedInteractable = nullptr;
	}

	// Invoke the interface pickup method
	IInteractable::Execute_OnPickup(CollectedInteractableActor, OwningCombatCharacter, this);
}

void ACustomPlayerController::UseInteractableActor()
{
	// only control one aircraft at a time
	if (OwningCombatCharacter->GetIsExitingVehicle() || ControlledVehicle != nullptr) {
		return;
	}

	auto Interactable = CollectedInteractableActor;

	// if has no collectable interaction then make the current support package available for interaction.
	if (!Interactable)
	{
		Interactable = CurrentSupportPackage;
	}

	if (UKismetSystemLibrary::IsValid(Interactable))
	{
		// Can use the interactable?
		auto CanUseInteractable = IInteractable::Execute_OnUseInteraction(Interactable, OwningCombatCharacter, this);

		// Cannot interact? 
		if (!CanUseInteractable)
		{
			// if the interactable is not the current support package, then try interacting with the support package.
			// Instances like this can occur if player just interacted with an objective but the objective cannot be used.
			if (Interactable != CurrentSupportPackage && UKismetSystemLibrary::IsValid(CurrentSupportPackage))
			{
				CanUseInteractable = IInteractable::Execute_OnUseInteraction(CurrentSupportPackage, OwningCombatCharacter, this);
			}
		}
	}

	CollectedInteractableActor = nullptr;
}

void ACustomPlayerController::SetControlledVehicle(AVehicleBase* InVehicle, bool IsContolled)
{
	if (!InVehicle) {
		return;
	}

	if (IsContolled)
	{
		// Unbind events of the previous controlled vehicle as they should not affect player anymore. 
		if (ControlledVehicle)
		{
			ControlledVehicle->OnVehicleDestroy.RemoveDynamic(this, &ACustomPlayerController::OnVehicleDestroy);
			ControlledVehicle->GetVehiclePathFollowerComponent()->OnVehiclePointReached.RemoveDynamic(this, &ACustomPlayerController::OnVehiclePointReached);
		}


		ControlledVehicle = InVehicle;

		// Remove inputs for this controller & character as we will be using aircraft inputs
		OwningCombatCharacter->DisableInput(this);
		AutoReceiveInput = EAutoReceiveInput::Disabled;
		GameHUDController->RemoveAllWidgets();
		ControlledVehicle->SetPlayerControl(this);


		ControlledVehicle->OnVehicleDestroy.AddDynamic(this, &ACustomPlayerController::OnVehicleDestroy);

		ControlledVehicle->GetVehiclePathFollowerComponent()->OnVehiclePointReached.AddDynamic(this, &ACustomPlayerController::OnVehiclePointReached);

		// drop the MG if using it so that the use of the interactable can be played with an animation
		OwningCombatCharacter->DropMountedGun();

		// if controlling aircraft then do not check for interactions
		GetWorldTimerManager().ClearTimer(THandler_CheckInteractable);
	}

	// update support package event for UI
	SupportPackages.RemoveAt(CurrentSupportPackageIndex);
	SortSupportPackages();
	OnSupportPackageUpdate.Broadcast(CurrentSupportPackage, CurrentSupportPackageIndex, false);
}

void ACustomPlayerController::AddSupportPackage(ASupportPackage* InSupportPackage)
{
	CurrentSupportPackage = Cast<ASupportPackage>(CollectedInteractableActor);

	// update support package event for UI
	SupportPackages.Add(CurrentSupportPackage);
	CurrentSupportPackageIndex = SupportPackages.Num() - 1;
	OnSupportPackageUpdate.Broadcast(CurrentSupportPackage, CurrentSupportPackageIndex, true);
}

/// <summary>
/// Update the current support package index
/// </summary>
void ACustomPlayerController::SortSupportPackages()
{
	if (SupportPackages.Num() > 0)
	{
		CurrentSupportPackageIndex = SupportPackages.Num() - 1;
		CurrentSupportPackage = SupportPackages[CurrentSupportPackageIndex];
	}
	else
	{
		CurrentSupportPackage = nullptr;
		CurrentSupportPackageIndex = -1;
	}
}

void ACustomPlayerController::SelectSupportPackage(int32 Index)
{
	int32 TargetPosition = SupportPackages.Num() - 1;

	// ignore if selecting the same index
	if (TargetPosition == Index) {
		return;
	}

	// swap last index with the selected index
	auto Temp = SupportPackages[TargetPosition];
	SupportPackages[TargetPosition] = SupportPackages[Index];
	SupportPackages[Index] = Temp;


	// update current support package
	CurrentSupportPackage = SupportPackages[TargetPosition];
	CurrentSupportPackageIndex = TargetPosition;

	if (PlaySupportPackageSound)
	{
		CurrentSupportPackage->PlayVoiceOverSound(PlayerFaction);
		CurrentSupportPackage->PlayInteractSound();
	}


	// if current collected interactable is a support package, it will still reference the previious support package. 
	// Either update the collected interactable to current support package or make it null
	CollectedInteractableActor = Cast<ASupportPackage>(CollectedInteractableActor) ? CurrentSupportPackage : CollectedInteractableActor;

	// dispatch event
	OnSupportPackageUpdate.Broadcast(CurrentSupportPackage, TargetPosition, true);

}

bool ACustomPlayerController::CanAddSupportPackages()
{
	if (SupportPackages.Num() < MaxSupportPackages) {
		return true;
	}

	return false;
}

void ACustomPlayerController::UseMountedGun()
{
	if (OwningCombatCharacter->GetMountedGun() == nullptr) {
		return;
	}

	// don't need to check for interaction if already using it
	GetWorldTimerManager().ClearTimer(THandler_CheckInteractable);

	// disable character input
	OwningCombatCharacter->DisableInput(this);


	if (OwningCombatCharacter->GetMountedGun()->GetCanExitMG())
	{
		// Display "Stop" message if using the mounted gun
		OnInteractionFound.Broadcast(OwningCombatCharacter->GetMountedGun()->GetStopUsingMessage().ToString(), InteractKeyDisplayName);
	}
}

void ACustomPlayerController::DropMountedGun()
{
	BeginCheckInteractable();
}


void ACustomPlayerController::ShowAircraftView()
{
	if (OwningCombatCharacter->GetVehicletSeat().OwningVehicle)
	{
		OwningCombatCharacter->GetVehicletSeat().OwningVehicle->SetPlayerControl(this, false, false);
	}
}
void ACustomPlayerController::HideAircraftView()
{
	if (OwningCombatCharacter->GetVehicletSeat().OwningVehicle == nullptr) {
		return;
	}

	if (OwningCombatCharacter->GetMountedGun())
	{
		OwningCombatCharacter->GetMountedGun()->SetPlayerControl(this, OwningCombatCharacter);
	}
	else
	{
		SetViewTargetWithBlend(OwningCombatCharacter);
	}
}

void ACustomPlayerController::RemoveVehicleControl()
{
	GetWorldTimerManager().ClearTimer(THandler_RemoveVehicleControlPost);

	if (HasGameEnded) {
		return;
	}

	// if the vehicle is destroyed, then the transition of going from vehicle to player camera will not exist and will happen in a sudden moment instead.
	// In this case, we do not go through the delay of doing RemoveVehicleControlPost().
	bool CanTransition = UHealthComponent::IsActorAlive(ControlledVehicle) && IsValid(ControlledVehicle);

	EndAim();
	EndFire();

	ControlledVehicle->RemovePlayerControl();
	ControlledVehicle = nullptr;


	if (CanTransition)
	{
		float Delay = 2.f;

		// Play a smooth transition from aircraft back to player rather than suddenly going back to player.
		SetViewTargetWithBlend(OwningCombatCharacter, Delay);

		// to renable player controls and other features after transitioning back to player character.
		GetWorldTimerManager().SetTimer(THandler_RemoveVehicleControlPost, this, &ACustomPlayerController::RemoveVehicleControlPost, Delay, true);
	}
	else
	{
		RemoveVehicleControlPost();
	}
}

void ACustomPlayerController::RemoveVehicleControlPost()
{
	if (!ShouldRespawn) {
		GameHUDController->AddPlayerWidgets();
	}

	OwningCombatCharacter->EnableInput(this);
	BeginCheckInteractable();

	GetWorldTimerManager().ClearTimer(THandler_RemoveVehicleControlPost);
}

#pragma region Commander Functions

void ACustomPlayerController::ClearInputHold()
{
	CurrentInputHoldTime = 0.0f;
	GetWorldTimerManager().ClearTimer(THandler_InputHeld);
}

void ACustomPlayerController::Recruit()
{
	if (OwningCommander == nullptr) {
		return;
	}

	OwningCommander->InteractWithOperative();
}


#pragma region Attack

void ACustomPlayerController::Attack()
{
	if (CurrentInputHoldTime < DesiredInputHoldTime)
	{
		CurrentInputHoldTime += .1f;
	}
	else
	{
		OwningCommander->Attack(true);

		// set it to desired amount before releasing the input key, 
		// as the release command or end command functions send the same command but for only for recruit
		CurrentInputHoldTime = DesiredInputHoldTime;
		GetWorldTimerManager().ClearTimer(THandler_InputHeld);
	}
}

void ACustomPlayerController::BeginAttackCommand()
{
	if (OwningCommander == nullptr) {
		return;
	}

	// incase another command key is pressed, clear the current time
	ClearInputHold();

	GetWorldTimerManager().SetTimer(THandler_InputHeld, this, &ACustomPlayerController::Attack, .1f, true);
}

void ACustomPlayerController::EndAttackCommand()
{
	if (OwningCommander == nullptr) {
		return;
	}

	if (CurrentInputHoldTime < DesiredInputHoldTime)
	{
		OwningCommander->Attack();
	}

	ClearInputHold();
}

#pragma endregion

#pragma region Defend

void ACustomPlayerController::Defend()
{
	if (CurrentInputHoldTime < DesiredInputHoldTime)
	{
		CurrentInputHoldTime += .1f;
	}
	else
	{
		OwningCommander->DefendArea(true);

		// set it to desired amount before releasing the input key, 
		// as the release command or end command functions send the same command but for only for recruit
		CurrentInputHoldTime = DesiredInputHoldTime;
		GetWorldTimerManager().ClearTimer(THandler_InputHeld);
	}
}


void ACustomPlayerController::BeginDefendCommand()
{
	if (OwningCommander == nullptr) {
		return;
	}

	// incase another command key is pressed, clear the current time
	ClearInputHold();

	GetWorldTimerManager().SetTimer(THandler_InputHeld, this, &ACustomPlayerController::Defend, .1f, true);
}

void ACustomPlayerController::EndDefendCommand()
{
	if (OwningCommander == nullptr) {
		return;
	}

	if (CurrentInputHoldTime < DesiredInputHoldTime)
	{
		OwningCommander->DefendArea();
	}

	ClearInputHold();
}

#pragma endregion

#pragma region Follow

void ACustomPlayerController::Follow()
{
	if (CurrentInputHoldTime < DesiredInputHoldTime)
	{
		CurrentInputHoldTime += .1f;
	}
	else
	{
		OwningCommander->FollowCommander(true);

		// set it to desired amount before releasing the input key, 
		// as the release command or end command functions send the same command but for only for recruit
		CurrentInputHoldTime = DesiredInputHoldTime;
		GetWorldTimerManager().ClearTimer(THandler_InputHeld);
	}
}

void ACustomPlayerController::BeginFollowCommand()
{
	if (OwningCommander == nullptr) {
		return;
	}

	// incase another command key is pressed, clear the current time
	ClearInputHold();

	GetWorldTimerManager().SetTimer(THandler_InputHeld, this, &ACustomPlayerController::Follow, .1f, true);
}

void ACustomPlayerController::EndFollowCommand()
{
	if (OwningCommander == nullptr) {
		return;
	}

	if (CurrentInputHoldTime < DesiredInputHoldTime)
	{
		OwningCommander->FollowCommander();
	}

	ClearInputHold();
}

#pragma endregion

#pragma endregion
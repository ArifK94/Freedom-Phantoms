#include "Controllers/CustomPlayerController.h"

#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"

#include "Weapons/Weapon.h"
#include "Weapons/WeaponBullet.h"
#include "Weapons/AmmoCrate.h"
#include "Weapons/MountedGun.h"

#include "Vehicles/Aircraft.h"

#include "Props/Interactable.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetInputLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/InputSettings.h"

ACustomPlayerController::ACustomPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	DesiredInputHoldTime = .5f;

	InteractionLength = 500.0f;

	AutoReceiveInput = EAutoReceiveInput::Player0;
}

void ACustomPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	FInputActionBinding PauseInput = InputComponent->BindAction("Pause", IE_Pressed, this, &ACustomPlayerController::PauseGame);
	PauseInput.bExecuteWhenPaused = true;

	InputComponent->BindAxis("Turn", this, &ACustomPlayerController::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ACustomPlayerController::TurnAtRate);

	InputComponent->BindAxis("LookUp", this, &ACustomPlayerController::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ACustomPlayerController::LookUpAtRate);

	InputComponent->BindAxis("MoveForward", this, &ACustomPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACustomPlayerController::MoveRight);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACustomPlayerController::BeginJump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACustomPlayerController::EndJump);

	InputComponent->BindAction("Aim", IE_Pressed, this, &ACustomPlayerController::BeginAim);
	InputComponent->BindAction("Aim", IE_Released, this, &ACustomPlayerController::EndAim);

	InputComponent->BindAction("Sprint", IE_Pressed, this, &ACustomPlayerController::BeginSprint);
	InputComponent->BindAction("Sprint", IE_Released, this, &ACustomPlayerController::EndSprint);

	InputComponent->BindAction("Crouch", IE_Pressed, this, &ACustomPlayerController::BeginCrouch);

	InputComponent->BindAction("TakeCover", IE_Pressed, this, &ACustomPlayerController::TakeCover);

	InputComponent->BindAction("Fire", IE_Pressed, this, &ACustomPlayerController::BeginFire);
	InputComponent->BindAction("Fire", IE_Released, this, &ACustomPlayerController::EndFire);

	// number keys
	InputComponent->BindAction("SwitchWeapons", IE_Pressed, this, &ACustomPlayerController::SwitchWeapon);
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

	if (OwningCombatCharacter) {
		// Grab the "Pickup Key" for it to be displayed on the UI 
		UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
		TArray <FInputActionKeyMapping> OutMappings;
		Settings->GetActionMappingByName("Pickup", OutMappings);
		if (OutMappings.Num() > 0)
		{
			InteractKeyDisplayName = UKismetInputLibrary::Key_GetDisplayName(OutMappings[0].Key);
		}
	}
}

void ACustomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AddUIWidgets();

	BeginCheckInteractable();

	OnInteractionFound.Broadcast("");

	if (OwningCombatCharacter)
	{
		OwningCombatCharacter->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ACustomPlayerController::OnCharacterHit);
		OwningCombatCharacter->OnCombatUpdated.AddDynamic(this, &ACustomPlayerController::OnCombatModeUpdated);
		OnCombatModeUpdated(OwningCombatCharacter);

		if (OwningCombatCharacter->GetAircraftSeat().OwningAircraft)
		{
			SetViewTargetWithBlend(OwningCombatCharacter, 0.0f);
		}
		
		// Character can be spawned in to use MG such as helicopter gunner
		UseMountedGun();
		
	}
}

void ACustomPlayerController::PauseGame()
{
	UWorld* World = GetWorld();

	if (!World) {
		return;
	}

	if (UGameplayStatics::IsGamePaused(World))
	{
		// resume game
		if (PauseMenuWidget != nullptr && PauseMenuWidget->IsInViewport())
		{
			PauseMenuWidget->RemoveFromParent();
		}

		SetShowMouseCursor(false);

		UGameplayStatics::SetGamePaused(World, false);
	}
	else
	{	// pause game
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

void ACustomPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OwningCombatCharacter == nullptr) {
		PrimaryActorTick.bCanEverTick = false;
		return;
	}
}

void ACustomPlayerController::AddUIWidgets()
{
	UWorld* World = GetWorld();

	if (!World) {
		return;
	}

	// Weapon Ammo
	if (WeaponAmmoWidgetClass)
	{
		WeaponAmmoWidget = CreateWidget<UUserWidget>(World, WeaponAmmoWidgetClass);

		if (WeaponAmmoWidget)
		{
			WeaponAmmoWidget->AddToViewport();
		}
	}

	// Weapon Crosshairs
	if (WeaponCrosshairhWidgetClass)
	{
		WeaponCrosshairWidget = CreateWidget<UUserWidget>(World, WeaponCrosshairhWidgetClass);

		if (WeaponCrosshairWidget)
		{
			WeaponCrosshairWidget->AddToViewport();
		}
	}

	if (HealthWidgetClass)
	{
		HealthWidget = CreateWidget<UUserWidget>(World, HealthWidgetClass);

		if (HealthWidget)
		{
			HealthWidget->AddToViewport();
		}
	}


	if (InteractWidgetClass)
	{
		InteractWidget = CreateWidget<UUserWidget>(World, InteractWidgetClass);

		if (InteractWidget)
		{
			InteractWidget->AddToViewport();
		}
	}


	// Commander UI
	if (OwningCommander)
	{
		OwningCommander->AddUIWidget();
	}
}

void ACustomPlayerController::OnCharacterHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == NULL || OtherActor == OwningCombatCharacter) {
		return;
	}

	// if hit an ammo create
	AAmmoCrate* AmmoCrate = Cast<AAmmoCrate>(OtherActor);
	if (AmmoCrate)
	{
		bool HasPrimaryReplenished = OwningCombatCharacter->GetPrimaryWeapon()->ReplenishAmmo();

		bool HasSecondaryReplenished = OwningCombatCharacter->GetSecondaryWeaponObj()->ReplenishAmmo();

		bool HasUnderBarrelReplenished = false;
		if (OwningCombatCharacter->GetUnderBarrelWeapon()) {
			OwningCombatCharacter->GetUnderBarrelWeapon()->ReplenishAmmo();
		}

		bool SuccessfulReplenish = false;
		if (HasPrimaryReplenished || HasSecondaryReplenished || HasUnderBarrelReplenished)
		{
			SuccessfulReplenish = true;
		}

		if (SuccessfulReplenish)
		{
			AmmoCrate->PlaySuccess();
		}
		else
		{
			AmmoCrate->PlayFailed();
		}
	}
}


void ACustomPlayerController::OnAircraftDestroy(AAircraft* CurrentControlledAircraft)
{
	AddUIWidgets();
	ControlledAircraft = nullptr;
	OwningCombatCharacter->EnableInput(this);
	BeginCheckInteractable();
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


void ACustomPlayerController::AddControllerPitchInput(float Val)
{
	if (Val == 0.f || !IsLocalPlayerController())
	{
		return;
	}

	if (ControlledAircraft) // if controlling an aircraft
	{
		ControlledAircraft->AddControllerPitchInput(Val);
	}
	else if (OwningCombatCharacter->GetAircraftSeat().OwningAircraft && !OwningCombatCharacter->IsUsingMountedWeapon()) // if in an aircraft
	{
		if (OwningCombatCharacter->IsInCombatMode())
		{
			OwningCombatCharacter->GetAircraftSeat().OwningAircraft->AddControllerPitchInput(Val, OwningCombatCharacter->GetAircraftSeat());
			OwningCombatCharacter->UpdateAimCamera();
		}
		else
		{
			OwningCombatCharacter->GetAircraftSeat().OwningAircraft->AddControllerPitchInput(Val, true); // Roam around aircraft view
		}
	}
	else if (OwningCombatCharacter->IsUsingMountedWeapon())	// When using MG
	{
		OwningCombatCharacter->GetMountedGun()->AddControllerPitchInput(Val);
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

	if (ControlledAircraft)
	{
		ControlledAircraft->AddControllerYawInput(Val);
	}
	else if (OwningCombatCharacter->GetAircraftSeat().OwningAircraft && !OwningCombatCharacter->IsUsingMountedWeapon())
	{
		if (OwningCombatCharacter->IsInCombatMode())
		{
			OwningCombatCharacter->GetAircraftSeat().OwningAircraft->AddControllerYawInput(Val, OwningCombatCharacter->GetAircraftSeat());
			OwningCombatCharacter->UpdateAimCamera();
		}
		else
		{
			OwningCombatCharacter->GetAircraftSeat().OwningAircraft->AddControllerYawInput(Val, true);
		}
	}
	else if (OwningCombatCharacter->IsUsingMountedWeapon())
	{
		OwningCombatCharacter->GetMountedGun()->AddControllerYawInput(Val);
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
	if (Value == 0.0f) {
		return;
	}

	if (!OwningCombatCharacter->InputEnabled()) {
		return;
	}

	if (!OwningCombatCharacter->IsTakingCover())
	{
		// find out which way is forward
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		OwningCombatCharacter->AddMovementInput(Direction, Value);
	}
	else
	{
		if (Value == -1.0f)
		{
			OwningCombatCharacter->EscapeCover();
		}
	}

}

void ACustomPlayerController::MoveRight(float Value)
{
	//if (!isTakingCover || isAtCoverCorner)
	//	RightInputValue = Value;

	if (Value == 0.0f) {
		return;
	}

	if (!OwningCombatCharacter->InputEnabled()) {
		return;
	}

	//if (isTakingCover && !isAtCoverCorner)
	//	RightInputValue = Value;

	if (!OwningCombatCharacter->IsTakingCover())
	{
		//find out which way is right
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		OwningCombatCharacter->AddMovementInput(Direction, Value);
	}
	else
	{
		OwningCombatCharacter->CoverMovement(Value);
	}
}

void ACustomPlayerController::BeginCrouch()
{
	OwningCombatCharacter->BeginCrouch();
}

void ACustomPlayerController::BeginJump()
{
	OwningCombatCharacter->Jump();
}

void ACustomPlayerController::EndJump()
{
	OwningCombatCharacter->StopJumping();
}

void ACustomPlayerController::BeginSprint()
{
	OwningCombatCharacter->BeginSprint();
}
void ACustomPlayerController::EndSprint()
{
	OwningCombatCharacter->EndSprint();
}

void ACustomPlayerController::BeginAim()
{
	if (ControlledAircraft) {
		return;
	}

	OwningCombatCharacter->BeginAim();

	if (OwningCombatCharacter->GetCurrentWeapon()) {
		OwningCombatCharacter->GetCurrentWeapon()->ChargeUp();
	}

}

void ACustomPlayerController::EndAim()
{
	OwningCombatCharacter->EndAim();

	if (OwningCombatCharacter->GetCurrentWeapon()) {
		OwningCombatCharacter->GetCurrentWeapon()->ChargeDown();
	}
}

void ACustomPlayerController::TakeCover()
{

}

void ACustomPlayerController::BeginFire()
{
	if (ControlledAircraft)
	{
		AWeapon* CurrentWeapon = ControlledAircraft->GetCurrentWeaponObj();
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
	if (ControlledAircraft)
	{
		AWeapon* CurrentWeapon = ControlledAircraft->GetCurrentWeaponObj();
		if (CurrentWeapon) {
			CurrentWeapon->StopFire();
		}
	}
	else
	{
		OwningCombatCharacter->EndFire();
	}
}

void ACustomPlayerController::SwitchWeapon()
{
	if (OwningCombatCharacter->IsUsingMountedWeapon()) {
		return;
	}

	if (ControlledAircraft)
	{
		ControlledAircraft->ChangeWeapon();
	}
	else
	{
		OwningCombatCharacter->BeginWeaponSwap();
	}
}


void ACustomPlayerController::ToggleThermalVision()
{
	if (ControlledAircraft)
	{
		ControlledAircraft->ChangeThermalVision();
	}
}

void ACustomPlayerController::BeginCheckInteractable()
{
	if (InteractWidget) {
		GetWorldTimerManager().SetTimer(THandler_CheckInteractable, this, &ACustomPlayerController::CheckInteractable, 0.2f, true);
	}
}

void ACustomPlayerController::CheckInteractable()
{
	if (OwningCombatCharacter == nullptr) {
		return;
	}

	FName ActionMessage;

	FHitResult OutHit;
	FVector Start = OwningCombatCharacter->FollowCamera->GetComponentLocation();

	FVector ForwardVector = OwningCombatCharacter->FollowCamera->GetForwardVector();
	FVector End = ((ForwardVector * InteractionLength) + Start);

	FCollisionQueryParams CollisionParams;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			AActor* TargetActor = OutHit.GetActor();

			// Interactable object?
			AInteractable* Interactable = Cast<AInteractable>(TargetActor);
			FocusedInteractable = Interactable;

			// Mounted Gun?
			MG = Cast<AMountedGun>(TargetActor);

			if (Interactable)
			{
				ActionMessage = Interactable->GetActionMessage();
			}
			else if (MG && MG->GetCanTraceInteraction())
			{
				// As we would need another UI message to display message to stop using the gun, 
				// we check if the current weapon is not the mounted gun, meaning the player is not using it
				// also check if not already in use
				if (OwningCombatCharacter->GetCurrentWeapon() != MG && MG->GetOwner() == nullptr)
				{
					ActionMessage = MG->GetPickupMessage();
				}
			}
			else
			{
				ActionMessage = "";
			}
		}
	}

	OnInteractionFound.Broadcast(ActionMessage);
}

void ACustomPlayerController::PickupInteractable()
{
	if (FocusedInteractable)
	{
		CurrentInteractable = FocusedInteractable;

		FocusedInteractable->SetActorHiddenInGame(true);
		FocusedInteractable->SetHidden(true);

		FocusedInteractable->SetActorEnableCollision(false);
		FocusedInteractable->SetActorTickEnabled(false);
	}
	else if (MG) 		// Use Mounted Gun
	{
		// Stop using the mounted gun if currently using it
		if (OwningCombatCharacter->GetCurrentWeapon() == OwningCombatCharacter->GetMountedGun())
		{
			if (OwningCombatCharacter->GetMountedGun()->GetCanExitMG())
			{
				OwningCombatCharacter->GetMountedGun()->RemovePlayerControl(this, OwningCombatCharacter);

				OwningCombatCharacter->DropMountedGun();
				MG = nullptr;

				BeginCheckInteractable();

				// renable character input
				OwningCombatCharacter->EnableInput(this);
			}
		}
		else
		{
			OwningCombatCharacter->SetMountedGun(MG);
			OwningCombatCharacter->UseMountedGun();
			UseMountedGun();
		}
	}
}

void ACustomPlayerController::UseInteractableActor()
{
	if (CurrentInteractable == nullptr) {
		return;
	}

	OwningCombatCharacter->SetMountedGun(nullptr);

	CurrentInteractable->BeginInteraction(OwningCombatCharacter, this);

	if (CurrentInteractable->GetAircraft())
	{
		ControlledAircraft = CurrentInteractable->GetAircraft();
		OwningCombatCharacter->DisableInput(this);
		ControlledAircraft->SetPlayerControl(this);
		ControlledAircraft->OnAircraftDestroy.AddDynamic(this, &ACustomPlayerController::OnAircraftDestroy);

		// if controlling aircraft then do not check for interactions
		GetWorldTimerManager().ClearTimer(THandler_CheckInteractable);
	}

	CurrentInteractable = nullptr;
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

	OwningCombatCharacter->GetMountedGun()->SetPlayerControl(this, OwningCombatCharacter);

	if (OwningCombatCharacter->GetMountedGun()->GetCanExitMG())
	{
		// Display "Stop" message if using the mounted gun
		OnInteractionFound.Broadcast(OwningCombatCharacter->GetMountedGun()->GetStopUsingMessage());
	}

}


void ACustomPlayerController::ShowAircraftView()
{
	if (OwningCombatCharacter->GetAircraftSeat().OwningAircraft)
	{
		OwningCombatCharacter->GetAircraftSeat().OwningAircraft->SetPlayerControl(this, false, false);
	}
}
void ACustomPlayerController::HideAircraftView()
{
	if (OwningCombatCharacter->GetAircraftSeat().OwningAircraft == nullptr) {
		return;
	}

	if (OwningCombatCharacter->GetMountedGun())
	{
		OwningCombatCharacter->GetMountedGun()->SetPlayerControl(this, OwningCombatCharacter);
	}
	else
	{
		SetViewTargetWithBlend(OwningCombatCharacter, 0.0f);
	}
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

	OwningCommander->Recruit();
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
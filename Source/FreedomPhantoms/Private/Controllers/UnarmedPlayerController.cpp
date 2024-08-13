#include "Controllers/UnarmedPlayerController.h"
#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"

void AUnarmedPlayerController::InitInputComponent()
{
	FInputActionBinding PauseInput = InputComponent->BindAction("Pause", IE_Pressed, this, &ACustomPlayerController::PauseGame);
	PauseInput.bExecuteWhenPaused = true;

	InputComponent->BindAxis("Turn", this, &ACustomPlayerController::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ACustomPlayerController::TurnAtRate);

	InputComponent->BindAxis("LookUp", this, &ACustomPlayerController::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ACustomPlayerController::LookUpAtRate);

	InputComponent->BindAxis("MoveForward", this, &ACustomPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACustomPlayerController::MoveRight);

	InputComponent->BindAction("Sprint", IE_Pressed, this, &ACustomPlayerController::ToggleSprint);

	InputComponent->BindAction("Crouch", IE_Pressed, this, &ACustomPlayerController::ToggleCrouch);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACustomPlayerController::BeginJump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACustomPlayerController::EndJump);

	InputComponent->BindAction("Pickup", IE_Pressed, this, &ACustomPlayerController::PickupInteractable);
}

void AUnarmedPlayerController::InitBeginPlayUncommon() {}

void AUnarmedPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	OwningCombatCharacter->HolsterWeapon();

	if (OwningCommander)
	{
		OwningCommander->SetCanSearchRecruits(false);
	}

	BeginCheckInteractable();
}

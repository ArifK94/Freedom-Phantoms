// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/UnarmedPlayerController.h"
#include "Characters/CombatCharacter.h"

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

	InputComponent->BindAction("Sprint", IE_Pressed, this, &ACustomPlayerController::BeginSprint);
	InputComponent->BindAction("Sprint", IE_Released, this, &ACustomPlayerController::EndSprint);

	InputComponent->BindAction("Crouch", IE_Pressed, this, &ACustomPlayerController::BeginCrouch);
}

void AUnarmedPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	OwningCombatCharacter->HolsterWeapon();
}

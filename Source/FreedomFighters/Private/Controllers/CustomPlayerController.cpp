#include "Controllers/CustomPlayerController.h"

#include "Managers/FactionManager.h"

#include "Characters/CombatCharacter.h"

#include "Weapons/Weapon.h"
#include "Weapons/WeaponBullet.h"

#include "Vehicles/Aircraft.h"

#include "Kismet/GameplayStatics.h"

#include "Blueprint/UserWidget.h"

ACustomPlayerController::ACustomPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACustomPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

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
}


void ACustomPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	OwningPawn = InPawn;

	OwningCombatCharacter = Cast<ACombatCharacter>(OwningPawn);
	AddUIWidgets();
}

void ACustomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SpawnAC130();
}

void ACustomPlayerController::AddUIWidgets()
{
	UWorld* World = GetWorld();

	if (!World) return;

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
}


void ACustomPlayerController::OnAircraftDestroy(AAircraft* CurrentControlledAircraft)
{
	AddUIWidgets();
	ControlledAircraft = nullptr;
	OwningCombatCharacter->EnableInput(this);
}


void ACustomPlayerController::AddControllerPitchInput(float Val)
{
	if (Val != 0.f && IsLocalPlayerController())
	{
		if (ControlledAircraft) {
			ControlledAircraft->AddControllerPitchInput(Val);
		}
		else
		{
			AddPitchInput(Val);
		}
	}
}

void ACustomPlayerController::AddControllerYawInput(float Val)
{
	if (Val != 0.f && IsLocalPlayerController())
	{
		if (ControlledAircraft) {
			ControlledAircraft->AddControllerYawInput(Val);
		}
		else
		{
			AddYawInput(Val);
		}
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
	if (Value != 0.0f)
	{
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
}

void ACustomPlayerController::MoveRight(float Value)
{
	//if (!isTakingCover || isAtCoverCorner)
	//	RightInputValue = Value;

	if (Value == 0.0f) {
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
	OwningCombatCharacter->BeginAim();
}

void ACustomPlayerController::EndAim()
{
	OwningCombatCharacter->EndAim();
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

void ACustomPlayerController::SpawnAC130()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwningCombatCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (OwningCombatCharacter->getFactionObj())
	{
		ControlledAircraft = GetWorld()->SpawnActor<AAircraft>(OwningCombatCharacter->getFactionObj()->GetAC130Class(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		OwningCombatCharacter->DisableInput(this);
		ControlledAircraft->SetPlayerControl(this);
		ControlledAircraft->OnAircraftDestroy.AddDynamic(this, &ACustomPlayerController::OnAircraftDestroy);
	}
}

void ACustomPlayerController::SwitchWeapon()
{
	if (ControlledAircraft)
	{
		ControlledAircraft->ChangeWeapon();
	}
}


void ACustomPlayerController::ToggleThermalVision()
{
	if (ControlledAircraft)
	{
		ControlledAircraft->ChangeThermalVision();
	}
}
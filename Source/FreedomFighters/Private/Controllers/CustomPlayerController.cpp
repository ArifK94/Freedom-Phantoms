#include "Controllers/CustomPlayerController.h"

#include "Managers/GameHUDController.h"
#include "Managers/FactionManager.h"

#include "Characters/CombatCharacter.h"

#include "Weapons/Weapon.h"
#include "Weapons/WeaponBullet.h"

#include "Vehicles/Aircraft.h"

#include "Components/InputComponent.h"

#include "Kismet/GameplayStatics.h"

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


}

void ACustomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	GameHUDController = Cast<AGameHUDController>(GetWorld()->GetFirstPlayerController()->GetHUD());

	SpawnAC130();
}

void ACustomPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

void ACustomPlayerController::BeginFire()
{
	if (ControlledAircraft)
	{
		CurrentWeapon = ControlledAircraft->GetCurrentWeaponObj();
		if (CurrentWeapon) {
			CurrentWeapon->StartFire();
		}
	}
	else
	{
		CurrentWeapon = OwningCombatCharacter->GetCurrentWeaponObj();
		OwningCombatCharacter->BeginFire();
	}
}

void ACustomPlayerController::EndFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
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
		ControlledAircraft->SetPlayerControl(this);
		GameHUDController->AddAC130ViewPort();
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
// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CombatCharacter.h"
#include "Managers/GameInstanceController.h"

#include "Accessories/Headgear.h"
#include "Accessories/Loadout.h"
#include "Accessories/NightVisionGoggle.h"

#include "Weapons/Weapon.h"
#include "FreedomFighters/FreedomFighters.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "Animation/AnimInstance.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include <array>

#include "GameFramework/Actor.h"



ACombatCharacter::ACombatCharacter()
{
	gameInstanceController = nullptr;

	currentWeaponObj = nullptr;
	primaryWeaponObj = nullptr;
	secondaryWeaponObj = nullptr;

	isAiming = false;
	isReloading = false;
	isEquippingWeapon = false;
	hasEquippedWeapon = false;
	isSwappingWeapon = false;
	isInCombatMode = false;

	MaxAimYawSprint = 180.0f;

}

//////////////////////////////////////////////////////////////////////////
// Input

void ACombatCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ACombatCharacter::BeginAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ACombatCharacter::EndAim);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACombatCharacter::BeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ACombatCharacter::EndFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ACombatCharacter::BeginReload);

	PlayerInputComponent->BindAction("EquipWeapon", IE_Pressed, this, &ACombatCharacter::BeginEquipWeapon);

	PlayerInputComponent->BindAction("SwitchWeapons", IE_Pressed, this, &ACombatCharacter::BeginWeaponSwap);

	PlayerInputComponent->BindAction("ToggleNightVision", IE_Pressed, this, &ACombatCharacter::ToggleNightVision);




}

void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (Headgears.Num() > 0)
	{
		SpawnHelmet();
	}

	if (Loadouts.Num() > 0)
	{
		SpawnLoadout();
	}


	if (gameInstanceController)
	{
		if (loadoutObj)
		{
			primaryWeaponObj = loadoutObj->SpawnPrimaryWeapon(loadoutObj->getLoadoutMesh(), this);
			secondaryWeaponObj = gameInstanceController->SpawnSecondaryWeapon(loadoutObj->getLoadoutMesh(), this);
		}

		else
		{
			//primaryWeaponObj = gameInstanceController->SpawnPrimaryWeapon(GetMesh(), this);
			//secondaryWeaponObj = gameInstanceController->SpawnSecondaryWeapon(GetMesh(), this);
		}

		if (primaryWeaponObj)
		{
			currentWeaponObj = primaryWeaponObj;
			BeginEquipWeapon();
		}
	}

}

void ACombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (currentWeaponObj)
	{
		UpdateCombatMode();

		UpdatePawnControl();

		UpdateSprint();

		UpdateFire();

		UpdateReload();

		if (isEquippingWeapon)
		{
			EndFire();
		}

		setCharacterRotation();
	}
}

AWeapon* ACombatCharacter::GetCurrentWeapon()
{
	return currentWeaponObj;
}

void ACombatCharacter::UpdateSprint()
{
	if (isSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = defaultMaxWalkSpeed * 2.5f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = defaultMaxWalkSpeed;
	}
}



void ACombatCharacter::setCharacterRotation()
{
	if (aimYaw > UKismetMathLibrary::Abs(MaxAimYawSprint))
	{
//		AActor::SetActorRotation();
	//	EndSprint();
	}
}

void ACombatCharacter::UpdatePawnControl()
{

	if (isInCombatMode && !isSprinting)
	{
		bUseControllerRotationYaw = true;
	}
	else
	{
		bUseControllerRotationYaw = false;
	}
}

void ACombatCharacter::BeginWeaponSwap()
{
	if (hasEquippedWeapon)
	{
		isSwappingWeapon = true;
	}
	else
	{
		if (currentWeaponObj == primaryWeaponObj)		// set secondary weapon
		{
			currentWeaponObj = secondaryWeaponObj;
		}
		else // set primary weapon
		{
			currentWeaponObj = primaryWeaponObj;
		}
	}
}


void ACombatCharacter::BeginEquipWeapon()
{
	isEquippingWeapon = true;
}

void ACombatCharacter::GrabWeapon()
{
	if (!hasEquippedWeapon)
	{
		setWeaponHand();
		hasEquippedWeapon = true;
	}
	else
	{
		unEquipWeapon();
		hasEquippedWeapon = false;
	}
}

void ACombatCharacter::EndEquipWeapon()
{
	isEquippingWeapon = false;
	isSwappingWeapon = false;
}


void ACombatCharacter::swapWeapon()
{
	if (isSwappingWeapon)
	{
		hasEquippedWeapon = false;

		if (currentWeaponObj == primaryWeaponObj)		// set secondary weapon
		{
			currentWeaponObj = secondaryWeaponObj;
		}
		else // set primary weapon
		{
			currentWeaponObj = primaryWeaponObj;
		}

		BeginEquipWeapon();
	}
}




void ACombatCharacter::ToggleNightVision()
{
	if (headgearObj)
	{
		if (headgearObj->getNightVision())
		{
			headgearObj->getNightVision()->ToggleVision();
		}
	}
}

void ACombatCharacter::setWeaponHand()
{
	currentWeaponObj->setWeaponSocket(GetMesh(), currentWeaponObj->getWeaponHandSocket());
}

void ACombatCharacter::unEquipWeapon()
{
	currentWeaponObj->setWeaponSocket(GetMesh(), currentWeaponObj->getHolsterSocket());
}


void ACombatCharacter::BeginFire()
{
	if (currentWeaponObj)
	{
		if (hasEquippedWeapon)
		{
			currentWeaponObj->StartFire();
		}
	}
}

void ACombatCharacter::EndFire()
{
	if (currentWeaponObj)
	{
		currentWeaponObj->StopFire();
	}
}

void ACombatCharacter::UpdateCombatMode()
{
	if (currentWeaponObj && hasEquippedWeapon)
	{
		if (isAiming || currentWeaponObj->isFiring)
			isInCombatMode = true;
		else
			isInCombatMode = false;
	}
}



void ACombatCharacter::UpdateFire()
{
	if (currentWeaponObj)
	{
		//if (isFiring && currentWeaponObj->getCurrentAmmo() <= 0)
		//{
		//	EndFire();
		//}

		if (!hasEquippedWeapon || isSwappingWeapon)
		{
			EndFire();
		}
	}

}


void ACombatCharacter::BeginAim()
{
	if (currentWeaponObj)
	{
		if (!hasEquippedWeapon)
		{
			BeginEquipWeapon();
		}
		else
		{
			isAiming = true;
		}

		EndSprint();
	}
}

void ACombatCharacter::EndAim()
{
	if (currentWeaponObj)
		isAiming = false;
}

void ACombatCharacter::BeginReload()
{
	if (currentWeaponObj)
	{
		currentWeaponObj->BeginReload();
	//	currentWeaponObj->SetClipSocket(GetMesh());
	}
}

void ACombatCharacter::EndReload()
{
	if (currentWeaponObj)
	{
		currentWeaponObj->EndReload();
	}
}

void ACombatCharacter::UpdateReload()
{
	if (currentWeaponObj)
	{
		isReloading = currentWeaponObj->isReloading;

		if (isReloading)
			isAiming = false;
	}
}

void ACombatCharacter::SpawnHelmet()
{
	UWorld* world = GetWorld();

	if (world)
	{
		int RandIndex = rand() % Headgears.Num();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		// Spawn a random helmet actor
		headgearObj = world->SpawnActor<AHeadgear>(Headgears[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (headgearObj)
		{
			headgearObj->SetOwner(this);
			//headgearObj->setMeshSocket(GetMesh());
			headgearObj->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "headgear_socket");
		}
	}
}

void ACombatCharacter::SpawnLoadout()
{
	UWorld* world = GetWorld();

	if (world)
	{
		int RandIndex = rand() % Loadouts.Num();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		// Spawn a random helmet actor
		loadoutObj = world->SpawnActor<ALoadout>(Loadouts[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (loadoutObj)
		{
			loadoutObj->SetOwner(this);
			loadoutObj->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			loadoutObj->getLoadoutMesh()->SetMasterPoseComponent(GetMesh());

		}
	}
}



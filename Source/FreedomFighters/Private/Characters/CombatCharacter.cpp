// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CombatCharacter.h"
#include "Managers/FactionManager.h"
#include "Managers/FactionManager.h"

#include "Accessories/Headgear.h"
#include "Accessories/Loadout.h"
#include "Accessories/NightVisionGoggle.h"

#include "Weapons/Weapon.h"
#include "Weapons/WeaponAttachmentManager.h"
#include "Weapons/WeaponTorchlight.h"
#include "Weapons/WeaponLaser.h"
#include "Weapons/Pistol.h"
#include "Weapons/WeaponSet.h"

#include "FreedomFighters/FreedomFighters.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "CustomComponents/HealthComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "Animation/AnimInstance.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"


#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "TimerManager.h"




ACombatCharacter::ACombatCharacter()
{
	currentWeaponObj = nullptr;
	primaryWeaponObj = nullptr;
	secondaryWeaponObj = nullptr;

	isAiming = false;
	isFiring = false;
	isReloading = false;
	isEquippingWeapon = false;
	hasEquippedWeapon = false;
	isSwappingWeapon = false;
	isInCombatMode = false;
	IsInAimOffSetRotation = false;
	HasPlayedReloadingSound = false;
	HasPlayedTargetFoundSound = false;

	MaxAimYawSprint = 180.0f;
	HandGuardAlpha = 0.0f;

	WeaponHandSocket = "weapon_hand";
	SecondaryWeaponHandSocket = "weapon_hand_secondary";
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

	//PlayerInputComponent->BindAction("EquipWeapon", IE_Pressed, this, &ACombatCharacter::BeginEquipWeapon);

	PlayerInputComponent->BindAction("SwitchWeapons", IE_Pressed, this, &ACombatCharacter::BeginWeaponSwap);

	PlayerInputComponent->BindAction("ToggleNightVision", IE_Pressed, this, &ACombatCharacter::ToggleNightVision);

	PlayerInputComponent->BindAction("ToggleUnderBarrel", IE_Pressed, this, &ACombatCharacter::ToggleUnderBarrelWeapon);

	PlayerInputComponent->BindAction("ToggleLaser", IE_Pressed, this, &ACombatCharacter::ToggleLaser);
	PlayerInputComponent->BindAction("ToggleLight", IE_Pressed, this, &ACombatCharacter::ToggleLight);
}

void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FactionClass)
		FactionObj = NewObject<UFactionManager>((UObject*)GetTransientPackage(), FactionClass);

	if (FactionObj)
	{
		FactionObj->Init(GetWorld());

		headgearObj = FactionObj->SpawnHelmet(GetMesh(), this);
		loadoutObj = FactionObj->SpawnLoadout(GetMesh(), this);


		if (loadoutObj)
		{
			primaryWeaponObj = loadoutObj->SpawnPrimaryWeapon(loadoutObj->getLoadoutMesh(), this);
			secondaryWeaponObj = FactionObj->getWeaponSetObj()->SpawnSecondaryWeapon(GetWorld(), loadoutObj->getLoadoutMesh(), this);
		}

		if (primaryWeaponObj)
		{
			currentWeaponObj = primaryWeaponObj;
			BeginEquipWeapon();
		}

		if (currentWeaponObj->getWeaponAttachmentObj() != NULL)
		{
			underBarrelWeaponObj = currentWeaponObj->getWeaponAttachmentObj()->getUnderBarrelWeaponObj();
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

		if (isEquippingWeapon || isDead)
		{
			EndFire();
		}


		UpdateHandGaurdIK();
		setCharacterRotation();
		disableSprint();


		if (isInCombatMode && CharacterSpeed > UKismetMathLibrary::Abs(0.0f))
		{
			isTakingCover = false;
		}
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
	if (isInCombatMode)
	{
		float x = 0.0f, y = 0.0f;

		auto controlYaw = 0.0f, actorYaw = 0.0f;

		UKismetMathLibrary::BreakRotator(GetControlRotation(), x, y, controlYaw);
		UKismetMathLibrary::BreakRotator(GetActorRotation(), x, y, actorYaw);

		auto ControlTargetRot = UKismetMathLibrary::MakeRotator(x, y, controlYaw);
		auto ActorTargetRot = UKismetMathLibrary::MakeRotator(x, y, actorYaw);

		FRotator Target = UKismetMathLibrary::NormalizedDeltaRotator(ControlTargetRot, ActorTargetRot);

		if (UKismetMathLibrary::Abs(Target.Yaw) >= 70.0f || IsInAimOffSetRotation)
		{
			IsInAimOffSetRotation = true;
		}

		if (IsInAimOffSetRotation)
		{
			FRotator MoveToTarget = FMath::RInterpTo(FRotator::ZeroRotator, Target, CurrentDeltaTime, 5.0f);
			AddActorWorldRotation(MoveToTarget);

			IsInAimOffSetRotation = !UKismetMathLibrary::NearlyEqual_FloatFloat(Target.Yaw, 0.0f, 2.0f);
		}
	}
}

void ACombatCharacter::disableSprint()
{
	float x = 0.0f, y = 0.0f;

	auto controlYaw = 0.0f, actorYaw = 0.0f;

	UKismetMathLibrary::BreakRotator(GetControlRotation(), x, y, controlYaw);
	UKismetMathLibrary::BreakRotator(GetActorRotation(), x, y, actorYaw);

	auto ControlTargetRot = UKismetMathLibrary::MakeRotator(x, y, controlYaw);
	auto ActorTargetRot = UKismetMathLibrary::MakeRotator(x, y, actorYaw);

	FRotator Target = UKismetMathLibrary::NormalizedDeltaRotator(ControlTargetRot, ActorTargetRot);

	if (UKismetMathLibrary::Abs(Target.Yaw) >= UKismetMathLibrary::Abs(MaxAimYawSprint) && isInCombatMode)
	{
		EndSprint();
	}
}

void ACombatCharacter::UpdatePawnControl()
{
	if (isInCombatMode && CharacterSpeed > 0.1f && !isSprinting)
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
		// swap weapons without the need of animations if not already equipped
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

	if (isReloading)
		isReloading = false;
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
		if (isReloading)
			isReloading = false;


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
	if (currentWeaponObj->IsA(APistol::StaticClass()))
	{
		currentWeaponObj->setWeaponSocket(GetMesh(), SecondaryWeaponHandSocket);
	}
	else
	{
		currentWeaponObj->setWeaponSocket(GetMesh(), WeaponHandSocket);
	}

}

void ACombatCharacter::unEquipWeapon()
{
	currentWeaponObj->setWeaponSocket(GetMesh(), currentWeaponObj->getHolsterSocket());
}


void ACombatCharacter::BeginFire()
{
	if (currentWeaponObj)
	{
		if (hasEquippedWeapon && !isReloading)
		{
			isFiring = true;
			currentWeaponObj->StartFire();
		}
	}
}

void ACombatCharacter::EndFire()
{
	if (currentWeaponObj)
	{
		currentWeaponObj->StopFire();
		isFiring = false;
	}
}

void ACombatCharacter::UpdateCombatMode()
{
	if (currentWeaponObj && hasEquippedWeapon && !isReloading)
	{
		if (isAiming || isFiring)
			isInCombatMode = true;
		else
			isInCombatMode = false;
	}
}



void ACombatCharacter::UpdateFire()
{
	if (currentWeaponObj)
	{
		if (!hasEquippedWeapon || isSwappingWeapon || isReloading)
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
	if (currentWeaponObj && !isReloading)
	{
		currentWeaponObj->BeginReload();
	}
}

void ACombatCharacter::EndReload()
{
	if (currentWeaponObj)
	{
		currentWeaponObj->EndReload();

		HasPlayedReloadingSound = false;
	}
}

void ACombatCharacter::UpdateReload()
{
	if (currentWeaponObj)
	{
		isReloading = currentWeaponObj->getIsReloading();

		if (currentWeaponObj->getIsReloading() && !HasPlayedReloadingSound)
		{
			HasPlayedReloadingSound = true;
			if (FactionObj != NULL && FactionObj->getSelectedVoiceClipSet().ReloadingSound != NULL)
			{
				VoiceAudioComponent->Sound = FactionObj->getSelectedVoiceClipSet().ReloadingSound;
				VoiceAudioComponent->Play();
			}
		}
	}
}


void ACombatCharacter::ToggleUnderBarrelWeapon()
{
	if (currentWeaponObj && !isReloading)
	{
		if (currentWeaponObj == underBarrelWeaponObj)
		{
			currentWeaponObj = primaryWeaponObj;
		}
		else
		{
			if (underBarrelWeaponObj != NULL)
			{
				currentWeaponObj = underBarrelWeaponObj;
			}
		}
	}
}

void ACombatCharacter::UpdateHandGaurdIK()
{
	if (currentWeaponObj)
	{
		currentWeaponObj->SetHandGuardIK(GetMesh());

		if (isInCombatMode && !isReloading)
		{
			HandGuardAlpha = 1.0f;
		}
		else
		{
			HandGuardAlpha = 0.0f;
		}

	}
}

void ACombatCharacter::ToggleLaser()
{
	if (currentWeaponObj)
	{
		currentWeaponObj->getWeaponAttachmentObj()->getLaserObj()->ToggleBeam();
	}
}

void ACombatCharacter::ToggleLight()
{
	if (currentWeaponObj)
	{
		currentWeaponObj->getWeaponAttachmentObj()->getTorchLight()->ToggleBeam();
	}
}

void ACombatCharacter::TargetFound()
{
	if (!HasPlayedTargetFoundSound && !isDead)
	{
		if (FactionObj != NULL && FactionObj->getSelectedVoiceClipSet().TargetFoundSound != NULL)
		{
			VoiceAudioComponent->Sound = FactionObj->getSelectedVoiceClipSet().TargetFoundSound;
			VoiceAudioComponent->Play();
			HasPlayedTargetFoundSound = true;
		}
	}

}

ACombatCharacter* ACombatCharacter::FindNearestFriendly()
{

	TArray<AActor*> allCombatChars;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatCharacter::StaticClass(), allCombatChars);
	AActor* ClosestAlly = nullptr;

	for (auto CurrentCombatant : allCombatChars)
	{
		if (CurrentCombatant != this)
		{
			UHealthComponent* CurrentHealth = Cast<UHealthComponent>(CurrentCombatant->GetComponentByClass(UHealthComponent::StaticClass()));

			bool IsAlive = CurrentHealth->getCurrentHealth() > 0.0f;
			bool isFriendly = UHealthComponent::IsFriendly(this, CurrentCombatant);

			if (isFriendly && IsAlive)
			{
				ClosestAlly = CurrentCombatant;

				if (CurrentCombatant->GetDistanceTo(this) < ClosestAlly->GetDistanceTo(this))
				{
					ClosestAlly = CurrentCombatant;
				}
			}

		}
	}

	if (ClosestAlly != nullptr)
	{
		return	Cast<ACombatCharacter>(ClosestAlly);
	}

	return NULL;
}

void ACombatCharacter::FriendlyKilled()
{
	if (!isDead)
	{
		if (FactionObj != NULL && FactionObj->getSelectedVoiceClipSet().FriendlyDownSound != NULL)
		{
			VoiceAudioComponent->Sound = FactionObj->getSelectedVoiceClipSet().FriendlyDownSound;
			VoiceAudioComponent->Play();
		}
	}
}

void ACombatCharacter::EnemyKilled()
{
	if (!HasPlayedEnemyKilledSound && !isDead)
	{
		if (FactionObj != NULL && FactionObj->getSelectedVoiceClipSet().EnemyDownSound != NULL)
		{
			VoiceAudioComponent->Sound = FactionObj->getSelectedVoiceClipSet().EnemyDownSound;
			VoiceAudioComponent->Play();
			HasPlayedEnemyKilledSound = true;

			GetWorldTimerManager().SetTimer(THandler_VoiceSoundReset, this, &ACombatCharacter::ResetVoiceSound, 5.0f, true, 0.0f);
		}
	}

}

void ACombatCharacter::ResetVoiceSound()
{
	GetWorldTimerManager().ClearTimer(THandler_VoiceSoundReset);

	HasPlayedEnemyKilledSound = false;
}

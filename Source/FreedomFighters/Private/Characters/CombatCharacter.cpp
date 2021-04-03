#include "Characters/CombatCharacter.h"
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

#include "Components/SphereComponent.h"
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

#include "Containers/Array.h"

#include "GameFramework/Controller.h"


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
	CanAutoReloadWeapon = false;
	isInCombatMode = false;
	IsInAimOffSetRotation = false;
	HasPlayedReloadingSound = false;
	HasPlayedTargetFoundSound = false;
	isUsingMountedWeapon = false;

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

	if (FactionClass) {
		FactionObj = NewObject<UFactionManager>((UObject*)GetTransientPackage(), FactionClass);
	}

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

	if (isDead) {
		return;
	}

	if (currentWeaponObj)
	{
		UpdateCombatMode();

		UpdatePawnControl();

		UpdateFire();

		UpdateReload();

		if (isEquippingWeapon || isDead)
		{
			EndFire();
		}


		UpdateHandGaurdIK();
		setCharacterRotation();
		disableSprint();
	}
}

void ACombatCharacter::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (isDead)
	{
		if (FactionObj != NULL && FactionObj->getSelectedVoiceClipSet().DeathSound != NULL)
		{
			VoiceAudioComponent->Sound = FactionObj->getSelectedVoiceClipSet().DeathSound;
			VoiceAudioComponent->Play();
		}

		if (currentWeaponObj) {
			EndFire();
			currentWeaponObj->getMeshComp()->SetCollisionProfileName(TEXT("Weapon"));
			currentWeaponObj->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			currentWeaponObj->getMeshComp()->SetSimulatePhysics(true);
		}
	}

	Super::OnHealthChanged(OwningHealthComp, Health, HealthDelta, DamageType, InstigatedBy, DamageCauser);
}

void ACombatCharacter::setCharacterRotation()
{
	if (isInCombatMode && !isTakingCover && !isInHelicopter)
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
		if (hasEquippedWeapon && !isReloading && !isFiring)
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
	if (currentWeaponObj && hasEquippedWeapon && !isReloading && !isRepellingDown)
	{
		if (isAiming || isFiring)
		{
			isInCombatMode = true;
		}
		else
		{
			isInCombatMode = false;
		}
	}
}



void ACombatCharacter::UpdateFire()
{
	if (currentWeaponObj == nullptr) {
		return;
	}

	if (isSwappingWeapon || isReloading || isRepellingDown || isDead) {
		EndFire();
	}

	if (CanAutoReloadWeapon && currentWeaponObj->getCurrentAmmo() <= 0) {
		BeginReload();
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
			currentWeaponObj->SetIsAiming(true);
		}

		EndSprint();
	}
}

void ACombatCharacter::EndAim()
{
	if (currentWeaponObj) {
		isAiming = false;
		currentWeaponObj->SetIsAiming(false);
	}
}

void ACombatCharacter::BeginReload()
{
	if (currentWeaponObj && !isReloading)
	{
		currentWeaponObj->BeginReload();
		isAiming = false;
		isFiring = false;
		currentWeaponObj->SetIsAiming(false);
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
	if (isDead || currentWeaponObj == nullptr) {
		return;
	}

	isReloading = currentWeaponObj->getIsReloading();

	//if (currentWeaponObj->getIsReloading() && !HasPlayedReloadingSound)
	//{
	//	HasPlayedReloadingSound = true;
	//	if (FactionObj != NULL && FactionObj->getSelectedVoiceClipSet().ReloadingSound != NULL)
	//	{
	//		VoiceAudioComponent->Sound = FactionObj->getSelectedVoiceClipSet().ReloadingSound;
	//		VoiceAudioComponent->Play();
	//	}
	//}

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
	// if mounted gun, then do not update hand IK
	if (currentWeaponObj == nullptr || isUsingMountedWeapon) {
		return;
	}
	currentWeaponObj->SetHandGuardIK(GetMesh(), RightHandSocket);

	if (isInCombatMode && !isReloading)
	{
		HandGuardAlpha = 1.0f;
	}
	else
	{
		HandGuardAlpha = 0.0f;
	}


}

void ACombatCharacter::ToggleLaser()
{
	if (currentWeaponObj && currentWeaponObj->getWeaponAttachmentObj() != nullptr && currentWeaponObj->getWeaponAttachmentObj()->getLaserObj() != nullptr)
	{
		currentWeaponObj->getWeaponAttachmentObj()->getLaserObj()->ToggleBeam();
	}
}

void ACombatCharacter::ToggleLight()
{
	if (currentWeaponObj && currentWeaponObj->getWeaponAttachmentObj() && currentWeaponObj->getWeaponAttachmentObj()->getTorchLight())
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

			if (isFriendly && CurrentHealth->IsAlive())
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

	return nullptr;
}

ACombatCharacter* ACombatCharacter::FindNearestEnemy(float TargetRange)
{
	TArray<AActor*> allCombatChars;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatCharacter::StaticClass(), allCombatChars);
	AActor* ClosestEnemy = nullptr;

	for (auto CurrentCombatant : allCombatChars)
	{
		if (CurrentCombatant != this)
		{
			UHealthComponent* CurrentHealth = Cast<UHealthComponent>(CurrentCombatant->GetComponentByClass(UHealthComponent::StaticClass()));

			bool IsAlive = CurrentHealth->getCurrentHealth() > 0.0f;
			bool isFriendly = UHealthComponent::IsFriendly(this, CurrentCombatant);

			if (!isFriendly && IsAlive)
			{
				ClosestEnemy = CurrentCombatant;

				auto DistanceDiff = (CurrentCombatant->GetActorLocation() - this->GetActorLocation()).Size();

				if (DistanceDiff < TargetRange)
				{
					ClosestEnemy = CurrentCombatant;
				}
			}

		}
	}

	if (ClosestEnemy != nullptr)
	{
		return	Cast<ACombatCharacter>(ClosestEnemy);
	}

	return nullptr;
}

void ACombatCharacter::FriendlyKilled()
{
	if (isDead) {
		return;
	}

	if (FactionObj != NULL && FactionObj->getSelectedVoiceClipSet().FriendlyDownSound != NULL)
	{
		VoiceAudioComponent->Sound = FactionObj->getSelectedVoiceClipSet().FriendlyDownSound;
		VoiceAudioComponent->Play();
	}

}

void ACombatCharacter::EnemyKilled()
{
	if (!isDead) {
		return;
	}

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

void ACombatCharacter::ShowCharacterOutline(bool CanShow)
{
	Super::ShowCharacterOutline(CanShow);

	if (loadoutObj != nullptr)
	{
		TArray<USkeletalMeshComponent*> LoadoutSkeletalMeshComponents;
		loadoutObj->GetComponents<USkeletalMeshComponent>(LoadoutSkeletalMeshComponents);
		for (int32 ComponentIdx = 0; ComponentIdx < LoadoutSkeletalMeshComponents.Num(); ++ComponentIdx)
		{
			auto currentComp = Cast<USkeletalMeshComponent>(LoadoutSkeletalMeshComponents[ComponentIdx]);
			currentComp->SetRenderCustomDepth(CanShow);
		}

		TArray<UStaticMeshComponent*> LoadoutStaticComponents;
		loadoutObj->GetComponents<UStaticMeshComponent>(LoadoutStaticComponents);
		for (int32 ComponentIdx = 0; ComponentIdx < LoadoutStaticComponents.Num(); ++ComponentIdx)
		{
			auto currentComp = Cast<UStaticMeshComponent>(LoadoutStaticComponents[ComponentIdx]);
			currentComp->SetRenderCustomDepth(CanShow);
		}
	}

	if (headgearObj != nullptr)
	{
		TArray<USkeletalMeshComponent*> HeadgearSkeletalMeshComponents;
		loadoutObj->GetComponents<USkeletalMeshComponent>(HeadgearSkeletalMeshComponents);
		for (int32 ComponentIdx = 0; ComponentIdx < HeadgearSkeletalMeshComponents.Num(); ++ComponentIdx)
		{
			auto currentComp = Cast<USkeletalMeshComponent>(HeadgearSkeletalMeshComponents[ComponentIdx]);
			currentComp->SetRenderCustomDepth(CanShow);
		}


		TArray<UStaticMeshComponent*> HeadgearStaticComponents;
		headgearObj->GetComponents<UStaticMeshComponent>(HeadgearStaticComponents);
		for (int32 ComponentIdx = 0; ComponentIdx < HeadgearStaticComponents.Num(); ++ComponentIdx)
		{
			auto currentComp = Cast<UStaticMeshComponent>(HeadgearStaticComponents[ComponentIdx]);
			currentComp->SetRenderCustomDepth(CanShow);
		}
	}
}

void ACombatCharacter::UseMountedGun(AWeapon* MountedGun)
{
	isUsingMountedWeapon = true;
	unEquipWeapon();
	MountedGun->SetOwner(this);
	currentWeaponObj = MountedGun;
}

void ACombatCharacter::DropMountedGun()
{
	isUsingMountedWeapon = false;

	// set to secondary so during weapon swap, it goes back to primary
	currentWeaponObj->SetOwner(nullptr);
	currentWeaponObj = primaryWeaponObj;
	BeginEquipWeapon();
	GrabWeapon();

}


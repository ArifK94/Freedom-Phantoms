#include "Characters/CombatCharacter.h"
#include "Accessories/Headgear.h"
#include "Accessories/Loadout.h"
#include "Accessories/NightVisionGoggle.h"
#include "Weapons/Weapon.h"
#include "Weapons/WeaponAttachmentManager.h"
#include "Weapons/WeaponTorchlight.h"
#include "Weapons/WeaponLaser.h"
#include "Weapons/Pistol.h"
#include "Weapons/PumpActionWeapon.h"
#include "Weapons/MountedGun.h"
#include "Weapons/WeaponBullet.h"
#include "CustomComponents/ObjectPoolComponent.h"
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


void ACombatCharacter::SetMountedGun(AWeapon* Mount)
{
	if (Mount)
	{
		MountedGun = Cast<AMountedGun>(Mount);
	}
	else
	{
		MountedGun = nullptr;
	}
}

void ACombatCharacter::SetPrimaryWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr)
	{
		primaryWeaponObj = Loadout->SpawnWeapon(WeaponsDataSet, true);
	}
	else
	{
		if (primaryWeaponObj)
		{
			primaryWeaponObj->Destroy();
		}

		primaryWeaponObj = Weapon;
		currentWeaponObj = primaryWeaponObj;
	}
	RetrieveWeaponAnimDataSet();

}

void ACombatCharacter::SetSecondaryWeapon(AWeapon* Weapon)
{

}

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
	HasPlayedTargetFoundSound = false;
	isUsingMountedWeapon = false;

	MaxAimYawSprint = 180.0f;
	HandGuardAlpha = 0.0f;

	WeaponHandSocket = "weapon_hand";
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

	RetrieveFactionDataSet();
	RetrieveWeaponDataSet();

	SpawnHelmet();
	SpawnLoadout();


	if (Loadout && WeaponsDataSet)
	{
		// get random weapon if not selected from main menu, this is should be null for AI characters
		if (primaryWeaponObj == nullptr)
		{
			primaryWeaponObj = Loadout->SpawnWeapon(WeaponsDataSet, true);
		}

		if (secondaryWeaponObj == nullptr)
		{
			secondaryWeaponObj = Loadout->SpawnWeapon(WeaponsDataSet, false);
		}
	}

	if (primaryWeaponObj)
	{
		currentWeaponObj = primaryWeaponObj;

		RegisterWeaponEvents(primaryWeaponObj, true);
	}
	else if (secondaryWeaponObj)
	{
		currentWeaponObj = secondaryWeaponObj;

		RegisterWeaponEvents(secondaryWeaponObj, true);
	}


	if (currentWeaponObj)
	{
		RetrieveWeaponAnimDataSet();
		BeginEquipWeapon();

		if (currentWeaponObj->getWeaponAttachmentObj() != NULL)
		{
			underBarrelWeaponObj = currentWeaponObj->getWeaponAttachmentObj()->getUnderBarrelWeaponObj();
		}
	}
}

void ACombatCharacter::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo)
{
	if (isDead)
	{
		if (GetVoiceClipsSet()->DeathSound != NULL)
		{
			VoiceAudioComponent->Sound = GetVoiceClipsSet()->DeathSound;
			VoiceAudioComponent->Play();
		}

		if (currentWeaponObj) {
			EndFire();
			currentWeaponObj->DropWeapon();
		}

		DropMountedGun();

		// Unregister the weapon events
		RegisterWeaponEvents(currentWeaponObj, false);
		RegisterWeaponEvents(primaryWeaponObj, false);
		RegisterWeaponEvents(secondaryWeaponObj, false);

	}

	Super::OnHealthChanged(OwningHealthComp, Health, HealthDelta, DamageType, InstigatedBy, DamageCauser, WeaponCauser, Bullet, HitInfo);
}

void ACombatCharacter::OnWeaponKillConfirm(int KillCount, bool IsSingleKill, bool IsDoubleKill, bool IsMultiKill)
{
	OnKillConfirm.Broadcast(KillCount);
}

void ACombatCharacter::RegisterWeaponEvents(AWeapon* Weapon, bool BindEvent)
{
	if (Weapon == nullptr) {
		return;
	}

	for (int i = 0; i < Weapon->GetObjectPoolComponent()->GetActorsInObjectPool().Num(); i++)
	{
		FObjectPoolParameters* ObjectPool = Weapon->GetObjectPoolComponent()->GetActorsInObjectPool()[i];
		AWeaponBullet* Bullet = Cast<AWeaponBullet>(ObjectPool->PoolableActor);

		if (Bullet)
		{
			if (BindEvent)
			{
				Bullet->OnKillConfirmed.AddDynamic(this, &ACombatCharacter::OnWeaponKillConfirm);
			}
			else
			{
				Bullet->OnKillConfirmed.RemoveDynamic(this, &ACombatCharacter::OnWeaponKillConfirm);
			}
		}
	}

	if (BindEvent)
	{
		if (CanAutoReloadWeapon) {
			Weapon->OnEmptyAmmoClip.AddDynamic(this, &ACombatCharacter::OnWeaponAmmoEmpty);
		}
	}
	else
	{
		Weapon->OnEmptyAmmoClip.RemoveDynamic(this, &ACombatCharacter::OnWeaponAmmoEmpty);
	}
}


void ACombatCharacter::RetrieveWeaponDataSet()
{
	if (WeaponsDatatable == nullptr) {
		return;
	}

	static const FString ContextString(TEXT("Weapons DataSet"));
	WeaponsDataSet = WeaponsDatatable->FindRow<FWeaponsSet>(WeaponsRowName, ContextString, true);
}

void ACombatCharacter::RetrieveWeaponAnimDataSet()
{
	if (WeaponsAnimationDatatable == nullptr) {
		return;
	}

	if (currentWeaponObj == nullptr) {
		return;
	}

	FName WeaponTypeName = currentWeaponObj->GetWeaponName();

	static const FString ContextString(TEXT("Weapons Animation DataSet"));
	FWeaponAnimSet* AnimSet = WeaponsAnimationDatatable->FindRow<FWeaponAnimSet>(WeaponTypeName, ContextString, true);

	// if anim set is null, maybe because the name has been provided for the weapon but the weapon row name does not exist in the data set
	// then try find the row based on the weapon type
	if (AnimSet == nullptr)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("WeaponType"), true);
		WeaponTypeName = EnumPtr->GetNameByValue((int64)currentWeaponObj->GetWeaponType());

		FString Left, Right;
		WeaponTypeName.ToString().Split(TEXT("::"), &Left, &Right);
		WeaponTypeName = *Right;

		AnimSet = WeaponsAnimationDatatable->FindRow<FWeaponAnimSet>(WeaponTypeName, ContextString, true);
	}

	WeaponAnimDataSet = AnimSet;

	if (WeaponAnimDataSet)
	{
		WeaponAnimDataSetEditor.StandArmed = WeaponAnimDataSet->StandArmed;
		WeaponAnimDataSetEditor.CrouchArmed = WeaponAnimDataSet->CrouchArmed;
		WeaponAnimDataSetEditor.StandAimingBS = WeaponAnimDataSet->StandAimingBS;
		WeaponAnimDataSetEditor.StartMoveBS = WeaponAnimDataSet->StartMoveBS;
		WeaponAnimDataSetEditor.StartMoveCombatBS = WeaponAnimDataSet->StartMoveCombatBS;
		WeaponAnimDataSetEditor.StopMoveBS = WeaponAnimDataSet->StopMoveBS;
		WeaponAnimDataSetEditor.CrouchAimingBS = WeaponAnimDataSet->CrouchAimingBS;
		WeaponAnimDataSetEditor.CrouchStartBS = WeaponAnimDataSet->CrouchStartBS;
		WeaponAnimDataSetEditor.CrouchStopBS = WeaponAnimDataSet->CrouchStopBS;
		WeaponAnimDataSetEditor.ProneAimingBS = WeaponAnimDataSet->ProneAimingBS;
		WeaponAnimDataSetEditor.DrawMontage = WeaponAnimDataSet->DrawMontage;
		WeaponAnimDataSetEditor.HolsterMontage = WeaponAnimDataSet->HolsterMontage;
		WeaponAnimDataSetEditor.Shooting = WeaponAnimDataSet->Shooting;
		WeaponAnimDataSetEditor.Reloading = WeaponAnimDataSet->Reloading;
		WeaponAnimDataSetEditor.AimOffsetStanding = WeaponAnimDataSet->AimOffsetStanding;
		WeaponAnimDataSetEditor.AimOffsetCrouching = WeaponAnimDataSet->AimOffsetCrouching;
		WeaponAnimDataSetEditor.AimOffsetProning = WeaponAnimDataSet->AimOffsetProning;
	}
}

void ACombatCharacter::RetrieveFactionDataSet()
{
	if (FactionDatatable == nullptr) {
		return;
	}

	static const FString ContextString(TEXT("Faction DataSet"));
	FactionDataSet = FactionDatatable->FindRow<FFaction>(FactionRowName, ContextString, true);
}

void ACombatCharacter::SpawnHelmet()
{
	if (GetAccessorySet() == nullptr) {
		return;
	}

	if (GetAccessorySet()->Headgears.Num() <= 0) {
		return;
	}

	if (GetWorld() == nullptr) {
		return;
	}


	int RandIndex = rand() % GetAccessorySet()->Headgears.Num();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


	// Spawn a random helmet actor
	Headgear = GetWorld()->SpawnActor<AHeadgear>(GetAccessorySet()->Headgears[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (Headgear)
	{
		Headgear->SetOwner(this);
		Headgear->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Headgear->GetParentSocket());
	}


}

void ACombatCharacter::SpawnLoadout()
{
	if (GetAccessorySet() == nullptr) {
		return;
	}

	if (GetAccessorySet()->Loadouts.Num() <= 0) {
		return;
	}

	if (GetWorld() == nullptr) {
		return;
	}

	int RandIndex = rand() % GetAccessorySet()->Loadouts.Num();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


	// Spawn a random helmet actor
	Loadout = GetWorld()->SpawnActor<ALoadout>(GetAccessorySet()->Loadouts[RandIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (Loadout)
	{
		Loadout->SetOwner(this);
		Loadout->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		if (Loadout->IsUseMasterPoseComponent()) {
			Loadout->GetMesh()->SetMasterPoseComponent(GetMesh());
		}
	}

}


void ACombatCharacter::BeginSprint()
{
	Super::BeginSprint();

	bUseControllerRotationYaw = false;
}

void ACombatCharacter::EndSprint()
{
	Super::EndSprint();

	if (isInCombatMode)
	{
		bUseControllerRotationYaw = true;
	}
}

// if in combat mode while sprinting and looking backwards then stop sprinting
// a mechanic observed in call of duty gameplay
void ACombatCharacter::DisableSprint()
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

void ACombatCharacter::StopCover()
{
	Super::StopCover();

	UpdateCombatMode();
}

void ACombatCharacter::UpdateCombatMode()
{
	if (currentWeaponObj == nullptr || !hasEquippedWeapon || isRepellingDown) {
		return;
	}

	if (isAiming || isFiring)
	{
		if (!isSprinting && !IsInAircraft && !isTakingCover && !isUsingMountedWeapon) {
			bUseControllerRotationYaw = true;
			GetCharacterMovement()->bOrientRotationToMovement = false;
		}

		isInCombatMode = true;
		SetHandGaurdIK(1.0f);
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		SetHandGaurdIK(0.0f);
		isInCombatMode = false;
		bUseControllerRotationYaw = false;
	}

	OnCombatUpdated.Broadcast(this);
}

void ACombatCharacter::BeginWeaponSwap()
{
	if (hasEquippedWeapon)
	{
		isSwappingWeapon = true;
		EndFire();
		UpdateCombatMode();

		if (WeaponAnimDataSet) {
			PlayAnimMontage(WeaponAnimDataSet->HolsterMontage);
		}
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
	if (isReloading) {
		EndReload();
	}

	if (currentWeaponObj == MountedGun) {
		return;
	}

	EndFire();

	isEquippingWeapon = true;

	if (WeaponAnimDataSet) {
		PlayAnimMontage(WeaponAnimDataSet->DrawMontage);
	}
}

void ACombatCharacter::GrabWeapon()
{
	if (currentWeaponObj == MountedGun) {
		return;
	}

	if (!hasEquippedWeapon)
	{
		currentWeaponObj->setWeaponSocket(GetMesh(), WeaponHandSocket);
		hasEquippedWeapon = true;
		UpdateCombatMode();
	}
	else
	{
		HolsterWeapon();
		hasEquippedWeapon = false;
		UpdateCombatMode();
	}
}

void ACombatCharacter::EndEquipWeapon()
{
	isEquippingWeapon = false;
	isSwappingWeapon = false;

	if (WeaponAnimDataSet) {
		StopAnimMontage(WeaponAnimDataSet->DrawMontage);
	}
}


void ACombatCharacter::swapWeapon()
{
	if (currentWeaponObj == MountedGun) {
		return;
	}

	if (!isSwappingWeapon) {
		return;
	}

	if (isReloading) {
		EndReload();
	}


	hasEquippedWeapon = false;
	UpdateCombatMode();

	if (currentWeaponObj == primaryWeaponObj)		// set secondary weapon
	{
		currentWeaponObj = secondaryWeaponObj;
	}
	else // set primary weapon
	{
		currentWeaponObj = primaryWeaponObj;
	}

	RetrieveWeaponAnimDataSet();
	BeginEquipWeapon();
}

/// <summary>
/// Replace primary & secondary weapons based on weapon type
/// End result will always be one primary as assault or LMG etc. and secondary will be pistol or other handgun types
/// </summary>
/// <param name="Weapon"></param>
void ACombatCharacter::PickupWeapon(AWeapon* Weapon)
{
	// Required for the weapon events
	Weapon->SetOwner(this);

	// register new weapon events
	RegisterWeaponEvents(Weapon, true);

	// update the primary or secondary weapon
	// based on the current weapon being used
	// unregister the weapon events
	if (currentWeaponObj == primaryWeaponObj)
	{
		RegisterWeaponEvents(primaryWeaponObj, false);
		primaryWeaponObj = Weapon;
	}
	else
	{
		RegisterWeaponEvents(secondaryWeaponObj, false);
		secondaryWeaponObj = Weapon;
	}


	// drop the current weapon
	currentWeaponObj->DropWeapon();

	// set the actor location of current to where the pickup weapon is
	currentWeaponObj->SetActorLocationAndRotation(Weapon->GetActorLocation(), Weapon->GetActorRotation());

	// assign new weapon to current weapon
	currentWeaponObj = Weapon;
	currentWeaponObj->setWeaponSocket(GetMesh(), WeaponHandSocket);

	RetrieveWeaponAnimDataSet();
}

void ACombatCharacter::HolsterWeapon()
{
	if (currentWeaponObj == MountedGun) {
		return;
	}

	currentWeaponObj->setWeaponSocket(Loadout->GetMesh(), currentWeaponObj->getHolsterSocket());
}


void ACombatCharacter::BeginFire()
{
	if (currentWeaponObj == nullptr || !hasEquippedWeapon) {
		return;
	}

	if (isReloading)
	{
		// Pump Action Weapons can fire if there is ammo
		if (Cast<APumpActionWeapon>(currentWeaponObj) && currentWeaponObj->getCurrentAmmo() > 0)
		{
			isReloading = false;
			EndReload();
		}
		else
		{
			return;
		}
	}

	isFiring = true;
	currentWeaponObj->StartFire();


	UpdateCombatMode();

	if (WeaponAnimDataSet) {
		PlayAnimMontage(WeaponAnimDataSet->Shooting);
	}

}

void ACombatCharacter::EndFire()
{
	if (currentWeaponObj)
	{
		currentWeaponObj->StopFire();
		isFiring = false;

		UpdateCombatMode();

		// in case character is not aiming
		if (!isAiming) {
			HandGuardAlpha = 0.0f;
		}

		if (WeaponAnimDataSet) {
			StopAnimMontage(WeaponAnimDataSet->Shooting);
		}
	}
}

// Begin Auto Reload
void ACombatCharacter::OnWeaponAmmoEmpty(AWeapon* Weapon)
{
	BeginReload();
	HandGuardAlpha = 0.0f;
}


void ACombatCharacter::BeginAim()
{
	if (currentWeaponObj == nullptr) {
		return;
	}

	if (!hasEquippedWeapon)
	{
		BeginEquipWeapon();
	}
	else
	{
		Super::BeginAim();
		currentWeaponObj->SetIsAiming(isAiming);
	}

	// if sprinting then stop
	if (isSprinting)
	{
		EndSprint();
	}

	UpdateCombatMode();
}

void ACombatCharacter::EndAim()
{
	Super::EndAim();

	if (currentWeaponObj) {
		currentWeaponObj->SetIsAiming(false);
	}

	UpdateCombatMode();
}

void ACombatCharacter::AimAutoRotation()
{
	if (isTakingCover || !IsInAircraft || !isInCombatMode) {
		return;
	}

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

void ACombatCharacter::BeginReload()
{
	if (currentWeaponObj == nullptr || isReloading || isSwappingWeapon) {
		return;
	}

	currentWeaponObj->BeginReload();
	isReloading = currentWeaponObj->getIsReloading();

	if (!isReloading) {
		return;
	}

	EndAim();
	EndFire();
	UpdateCombatMode();

	if (WeaponAnimDataSet) {
		PlayAnimMontage(WeaponAnimDataSet->Reloading);
	}

	if (GetVoiceClipsSet()->ReloadingSound != NULL)
	{
		VoiceAudioComponent->Sound = GetVoiceClipsSet()->ReloadingSound;
		VoiceAudioComponent->Play();
	}
}

void ACombatCharacter::EndReload()
{
	if (currentWeaponObj)
	{
		currentWeaponObj->EndReload();
		isReloading = currentWeaponObj->getIsReloading();
		UpdateCombatMode();

		if (WeaponAnimDataSet) {
			StopAnimMontage(WeaponAnimDataSet->Reloading);
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

void ACombatCharacter::SetHandGaurdIK(float Alpha)
{
	// if mounted gun, then do not update hand IK
	if (currentWeaponObj == nullptr || isUsingMountedWeapon) {
		return;
	}
	currentWeaponObj->SetHandGuardIK(GetMesh(), RightHandSocket);

	HandGuardAlpha = Alpha;
}

void ACombatCharacter::ToggleNightVision()
{
	if (Headgear)
	{
		if (Headgear->getNightVision())
		{
			Headgear->getNightVision()->ToggleVision();
		}
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
		if (GetVoiceClipsSet()->TargetFoundSound != NULL)
		{
			VoiceAudioComponent->Sound = GetVoiceClipsSet()->TargetFoundSound;
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

			if (!isFriendly && IsAlive && CurrentHealth->GetSelectedFaction() != TeamFaction::Neutral)
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

	if (GetVoiceClipsSet()->FriendlyDownSound != NULL)
	{
		VoiceAudioComponent->Sound = GetVoiceClipsSet()->FriendlyDownSound;
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
		if (GetVoiceClipsSet()->EnemyDownSound != NULL)
		{
			VoiceAudioComponent->Sound = GetVoiceClipsSet()->EnemyDownSound;
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

	if (Loadout != nullptr)
	{
		TArray<USkeletalMeshComponent*> LoadoutSkeletalMeshComponents;
		Loadout->GetComponents<USkeletalMeshComponent>(LoadoutSkeletalMeshComponents);
		for (int32 ComponentIdx = 0; ComponentIdx < LoadoutSkeletalMeshComponents.Num(); ++ComponentIdx)
		{
			auto currentComp = Cast<USkeletalMeshComponent>(LoadoutSkeletalMeshComponents[ComponentIdx]);
			currentComp->SetRenderCustomDepth(CanShow);
		}

		TArray<UStaticMeshComponent*> LoadoutStaticComponents;
		Loadout->GetComponents<UStaticMeshComponent>(LoadoutStaticComponents);
		for (int32 ComponentIdx = 0; ComponentIdx < LoadoutStaticComponents.Num(); ++ComponentIdx)
		{
			auto currentComp = Cast<UStaticMeshComponent>(LoadoutStaticComponents[ComponentIdx]);
			currentComp->SetRenderCustomDepth(CanShow);
		}
	}

	if (Headgear != nullptr)
	{
		TArray<USkeletalMeshComponent*> HeadgearSkeletalMeshComponents;
		Headgear->GetComponents<USkeletalMeshComponent>(HeadgearSkeletalMeshComponents);
		for (int32 ComponentIdx = 0; ComponentIdx < HeadgearSkeletalMeshComponents.Num(); ++ComponentIdx)
		{
			auto currentComp = Cast<USkeletalMeshComponent>(HeadgearSkeletalMeshComponents[ComponentIdx]);
			currentComp->SetRenderCustomDepth(CanShow);
		}


		TArray<UStaticMeshComponent*> HeadgearStaticComponents;
		Headgear->GetComponents<UStaticMeshComponent>(HeadgearStaticComponents);
		for (int32 ComponentIdx = 0; ComponentIdx < HeadgearStaticComponents.Num(); ++ComponentIdx)
		{
			auto currentComp = Cast<UStaticMeshComponent>(HeadgearStaticComponents[ComponentIdx]);
			currentComp->SetRenderCustomDepth(CanShow);
		}
	}
}

void ACombatCharacter::UseMountedGun()
{
	if (MountedGun == nullptr) {
		return;
	}

	// Ignore the mounted gun, so that it does push away the character from the MG when using it
	GetCapsuleComponent()->IgnoreActorWhenMoving(MountedGun, true);

	EndFire();
	EndAim();

	isUsingMountedWeapon = true;
	HolsterWeapon();
	MountedGun->SetOwner(this);
	MountedGun->SetPotentialOwner(this);
	currentWeaponObj = MountedGun;

	RegisterWeaponEvents(MountedGun, true);

	RetrieveWeaponAnimDataSet();


	if (MountedGun->GetAdjustBehindMG())
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;


		//UKismetSystemLibrary::MoveComponentTo(
		//	GetCapsuleComponent(),
		//	MountedGun->GetCharacterStandPos(),
		//	MountedGun->GetCharacterStandRot(),
		//	false,
		//	false,
		//	.2f,
		//	false,
		//	EMoveComponentAction::Type::Move,
		//	LatentInfo
		//);


		AttachToComponent(MountedGun->getMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, MountedGun->GetCharacterPositionSocket());
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	// in case character was spawned to use MG
	hasEquippedWeapon = true;
}

void ACombatCharacter::DropMountedGun(bool ClearMG)
{
	if (MountedGun == nullptr) {
		return;
	}

	EndFire();
	EndAim();

	// Unregister the kill event for the MG
	RegisterWeaponEvents(MountedGun, false);

	isUsingMountedWeapon = false;

	MountedGun->DropWeapon();

	// Reassign to collide with the MG again
	GetCapsuleComponent()->IgnoreActorWhenMoving(MountedGun, false);

	if (ClearMG) {
		MountedGun->SetOwner(nullptr);
		MountedGun->SetPotentialOwner(nullptr);
		MountedGun = nullptr;
	}

	currentWeaponObj = primaryWeaponObj;

	RetrieveWeaponAnimDataSet();
	BeginEquipWeapon();
	GrabWeapon();
}

void ACombatCharacter::SetIsRepellingDown(bool IsRappelling)
{
	Super::SetIsRepellingDown(IsRappelling);

	EndAim();
	EndFire();
	UpdateCombatMode();
}

void ACombatCharacter::PlayDeathAnim(AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo)
{
	if (isUsingMountedWeapon && !Bullet->IsExplosive())
	{
		DeathAnimationAsset = DeathAnimation->MountedGuns[rand() % DeathAnimation->MountedGuns.Num()];
	}
	else
	{
		Super::PlayDeathAnim(WeaponCauser, Bullet, HitInfo);
	}
}

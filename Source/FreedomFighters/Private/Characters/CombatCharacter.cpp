#include "Characters/CombatCharacter.h"
#include "Accessories/Headgear.h"
#include "Accessories/Loadout.h"
#include "Accessories/NightVisionGoggle.h"
#include "Weapons/Weapon.h"
#include "Weapons/Pistol.h"
#include "Weapons/PumpActionWeapon.h"
#include "Weapons/MountedGun.h"
#include "Weapons/WeaponBullet.h"
#include "CustomComponents/ObjectPoolComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
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
	if (!Weapon) {
		return;
	}

	if (primaryWeaponObj)
	{
		primaryWeaponObj->Destroy();
	}

	primaryWeaponObj = Weapon;
	currentWeaponObj = primaryWeaponObj;

	LoadoutType LoadoutType = LoadoutType::Assault;

	switch (primaryWeaponObj->GetWeaponType())
	{
	case WeaponType::LMG:
		LoadoutType = LoadoutType::LMG;
		break;
	case WeaponType::Shotgun:
		LoadoutType = LoadoutType::Shotgun;
		break;
	case WeaponType::SMG:
		LoadoutType = LoadoutType::SMG;
		break;
	}

	// respawn loadout to match weapon type
	SpawnLoadout(LoadoutType, true);

	RetrieveWeaponAnimDataSet();
}

void ACombatCharacter::SetSecondaryWeapon(AWeapon* Weapon)
{
	if (secondaryWeaponObj)
	{
		secondaryWeaponObj->Destroy();
	}

	secondaryWeaponObj = Weapon;

	if (Loadout)
	{
		Loadout->HolsterWeapon(secondaryWeaponObj);
	}
}

ACombatCharacter::ACombatCharacter()
{
	TeamFactionComponent = CreateDefaultSubobject<UTeamFactionComponent>(TEXT("TeamFactionComponent"));

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
	isUsingMountedWeapon = false;

	MaxAimYawSprint = 180.0f;
	HandGuardAlpha = 0.0f;

	WeaponHandSocket = "weapon_hand";
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
	}
	else if (secondaryWeaponObj)
	{
		currentWeaponObj = secondaryWeaponObj;
	}

	RegisterWeaponEvents(primaryWeaponObj, true);
	RegisterWeaponEvents(secondaryWeaponObj, true);


	if (currentWeaponObj)
	{
		RetrieveWeaponAnimDataSet();
		BeginEquipWeapon();
	}
}

void ACombatCharacter::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	Super::OnHealthUpdate(InHealthParameters);

	if (!HealthComp->IsAlive())
	{
		TeamFactionComponent->SetCompActive(false);

		// check if not using mounted gun as the weapon will be dropped and simulating physics
		if (currentWeaponObj && currentWeaponObj != MountedGun) {
			EndFire();
			currentWeaponObj->DropWeapon(true, true);
		}

		DropMountedGun();

		// Unregister the weapon events
		RegisterWeaponEvents(currentWeaponObj, false);
		RegisterWeaponEvents(primaryWeaponObj, false);
		RegisterWeaponEvents(secondaryWeaponObj, false);

		// Inform nearest ally of death
		auto nearestFriendly = FindNearestFriendly();

		if (nearestFriendly)
		{
			nearestFriendly->FriendlyKilled();
		}
	}
}

void ACombatCharacter::OnWeaponKillConfirm(int KillCount, bool IsSingleKill, bool IsDoubleKill, bool IsMultiKill)
{
	EnemyKilled();
	OnKillConfirm.Broadcast(KillCount);
}

// Begin Auto Reload
void ACombatCharacter::OnWeaponAmmoEmpty(AWeapon* Weapon)
{
	BeginReload();
	HandGuardAlpha = 0.0f;
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

void ACombatCharacter::SpawnLoadout(LoadoutType LoadoutType, bool SpecifyType)
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

	if (Loadout) {
		Loadout->Destroy();
	}

	int RandIndex = rand() % GetAccessorySet()->Loadouts.Num();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TSubclassOf<ALoadout> LoadoutClass = NULL;

	if (SpecifyType)
	{
		switch (LoadoutType)
		{
		case LoadoutType::SMG:
			LoadoutClass = GetAccessorySet()->SMGLoadout;
			break;
		case LoadoutType::Shotgun:
			LoadoutClass = GetAccessorySet()->ShotgunLoadout;
			break;
		case LoadoutType::LMG:
			LoadoutClass = GetAccessorySet()->LMGLoadout;
			break;
		default: // Assault loadout by default
			LoadoutClass = GetAccessorySet()->AssaultLoadout;
			break;
		}
	}

	if (!LoadoutClass)
	{
		LoadoutClass = GetAccessorySet()->Loadouts[RandIndex];
	}

	// Spawn a random helmet actor
	Loadout = GetWorld()->SpawnActor<ALoadout>(LoadoutClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

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

void ACombatCharacter::UpdateCharacterMovement()
{
	Super::UpdateCharacterMovement();

	// update the bUseControllerRotationYaw by ending the sprint
	// firing a weapon can be allowed during sprinting so need to check if character is not moving 
	// aiming stops sprinting by default
	if (isAiming || (!IsCharacterMoving() && isFiring)) {
		EndSprint();
	}

	UpdateCombatMode();
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
		if (!isSprinting && !IsInAircraft && !isTakingCover && !isUsingMountedWeapon)
		{
			bUseControllerRotationYaw = true;
			GetCharacterMovement()->bOrientRotationToMovement = false;
		}
		else
		{
			bUseControllerRotationYaw = false;
			GetCharacterMovement()->bOrientRotationToMovement = true;
		}

		isInCombatMode = true;
		SetHandGaurdIK(1.0f);
	}
	else
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		SetHandGaurdIK(0.0f);
		isInCombatMode = false;
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

	isReloading = false;
	isSwappingWeapon = false;
	EndFire();
	EndAim();
	UpdateCombatMode();

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
	if (currentWeaponObj == nullptr || !hasEquippedWeapon || isSwappingWeapon) {
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

	PlayVoiceSound(GetVoiceClipsSet()->ReloadingSound);
}

void ACombatCharacter::EndReload()
{
	if (currentWeaponObj == nullptr) {
		return;
	}

	currentWeaponObj->EndReload();
	isReloading = currentWeaponObj->getIsReloading();
	UpdateCombatMode();

	if (WeaponAnimDataSet) {
		StopAnimMontage(WeaponAnimDataSet->Reloading);
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

ACombatCharacter* ACombatCharacter::FindNearestFriendly()
{
	TArray<AActor*> allCombatChars;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatCharacter::StaticClass(), allCombatChars);
	ACombatCharacter* ClosestAlly = nullptr;

	for (int i = 0; i < allCombatChars.Num(); i++)
	{
		auto CurrentActor = allCombatChars[i];

		if (CurrentActor == this) {
			continue;
		}

		auto CurrentCombatant = Cast<ACombatCharacter>(CurrentActor);
		bool isFriendly = UTeamFactionComponent::IsFriendly(this, CurrentCombatant);

		if (isFriendly)
		{
			if (ClosestAlly == nullptr)
			{
				ClosestAlly = CurrentCombatant;
			}
			else
			{
				if (CurrentCombatant->GetDistanceTo(this) < ClosestAlly->GetDistanceTo(this))
				{
					ClosestAlly = CurrentCombatant;
				}
			}
		}
	}

	return ClosestAlly;
}

void ACombatCharacter::FriendlyKilled()
{
	PlayVoiceSound(GetVoiceClipsSet()->FriendlyDownSound);
}

void ACombatCharacter::EnemyKilled()
{
	// don't need to constantly repeat the voice clip after each kill
	if (HasPlayedEnemyKilledSound) {
		return;
	}

	PlayVoiceSound(GetVoiceClipsSet()->EnemyDownSound);

	HasPlayedEnemyKilledSound = true;

	GetWorldTimerManager().SetTimer(THandler_VoiceSoundReset, this, &ACombatCharacter::ResetVoiceSound, 5.0f, false);
}

void ACombatCharacter::ResetVoiceSound()
{
	GetWorldTimerManager().ClearTimer(THandler_VoiceSoundReset);

	HasPlayedEnemyKilledSound = false;
}

void ACombatCharacter::UseMountedGun()
{
	if (MountedGun == nullptr) {
		return;
	}

	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
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

void ACombatCharacter::PlayDeathAnim(FHealthParameters InHealthParameters)
{
	if (isUsingMountedWeapon && !InHealthParameters.IsExplosive)
	{
		DeathAnimationAsset = DeathAnimation->MountedGuns[rand() % DeathAnimation->MountedGuns.Num()];
	}
	else
	{
		Super::PlayDeathAnim(InHealthParameters);
	}
}

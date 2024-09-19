#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"
#include "Accessories/Headgear.h"
#include "Accessories/Loadout.h"
#include "Accessories/NightVisionGoggle.h"
#include "Weapons/Weapon.h"
#include "Weapons/Pistol.h"
#include "Weapons/PumpActionWeapon.h"
#include "Weapons/ThrowableWeapon.h"
#include "Weapons/MountedGun.h"
#include "Weapons/Projectile.h"
#include "CustomComponents/ObjectPoolComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "Managers/GameModeManager.h"
#include "FreedomPhantoms/FreedomPhantoms.h"

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
	KillCounter = 0;

	WeaponHandSocket = "weapon_hand";
	WeaponHandThrowablesSocket = "weapon_throwable_hand";
}

void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	RetrieveFactionDataSet();
	RetrieveWeaponDataSet();

	SpawnHelmet();
	SpawnLoadout();

	if (primaryWeaponObj) {
		primaryWeaponObj->SetOwner(this);
	}

	if (secondaryWeaponObj) {
		secondaryWeaponObj->SetOwner(this);
	}

	if (GrenadeWeapon) {
		GrenadeWeapon->SetOwner(this);
	}

	if (Loadout)
	{
		// get random weapon if not selected from main menu, this is should be null for AI characters
		if (primaryWeaponObj == nullptr)
		{
			primaryWeaponObj = PrimaryWeaponClass ? Loadout->SpawnWeapon(PrimaryWeaponClass, true) : Loadout->SpawnWeapon(WeaponsDataSet, true);
		}

		if (secondaryWeaponObj == nullptr)
		{
			secondaryWeaponObj = SecondaryWeaponClass ? Loadout->SpawnWeapon(SecondaryWeaponClass, false) : Loadout->SpawnWeapon(WeaponsDataSet, false);
		}

		if (GrenadeWeapon == nullptr)
		{
			GrenadeWeapon = Loadout->SpawnGrenade(WeaponsDataSet);
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
	RegisterWeaponEvents(Cast<AWeapon>(GrenadeWeapon), true);


	if (currentWeaponObj) {
		RetrieveWeaponAnimDataSet();
		BeginEquipWeapon();
	}
}

void ACombatCharacter::Init()
{
	Super::Init();

	isAiming = false;
	isFiring = false;
	isReloading = false;
	isEquippingWeapon = false;
	hasEquippedWeapon = false;
	isSwappingWeapon = false;
	isInCombatMode = false;
	isUsingMountedWeapon = false;
}

void ACombatCharacter::SetDefaultState()
{
	Super::SetDefaultState();

	EndFire();
	EndReload();
	EndEquipWeapon();
	DropMountedGun();
}

FString ACombatCharacter::GetKeyDisplayName_Implementation()
{
	return FString();
}

FString ACombatCharacter::OnInteractionFound_Implementation(APawn* InPawn, AController* InController)
{
	auto Commander = Cast<ACommanderCharacter>(InPawn);

	if (!Commander) {
		return FString();
	}

	if (Commander == this) {
		return FString();
	}

	if (Commander->GetPotentialRecruit()) {
		return Commander->GetCurrentMessage().ToString();
	}
	return FString();
}

AActor* ACombatCharacter::OnPickup_Implementation(APawn* InPawn, AController* InController)
{
	auto Commander = Cast<ACommanderCharacter>(InPawn);

	if (!Commander) {
		return nullptr;
	}

	Commander->InteractWithOperative();

	return nullptr;
}

bool ACombatCharacter::OnUseInteraction_Implementation(APawn* InPawn, AController* InController)
{
	return false;
}

bool ACombatCharacter::CanInteract_Implementation(APawn* InPawn, AController* InController)
{
	auto Commander = Cast<ACommanderCharacter>(InPawn);

	if (!Commander) {
		return false;
	}

	if (Commander == this) {
		return false;
	}


	// To allow commander to revive if max recruits reached
	if (HealthComp->GetIsWounded()) {
		return Commander->GetCanSearchRecruits();
	}
	else {
		return Commander->GetCanRecruit();
	}
}


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

	Weapon->SetOwner(this);
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
	RegisterWeaponEvents(primaryWeaponObj, true);
	RetrieveWeaponAnimDataSet();

	// as the previous loadout will be destroyed, the secondary weapon will be attached to no parent, this is need to be attached to the new loadout.
	Loadout->HolsterWeapon(secondaryWeaponObj);
}

void ACombatCharacter::SetSecondaryWeapon(AWeapon* Weapon)
{
	if (secondaryWeaponObj) {
		secondaryWeaponObj->Destroy();
	}

	Weapon->SetOwner(this);
	secondaryWeaponObj = Weapon;

	if (Loadout) {
		Loadout->HolsterWeapon(secondaryWeaponObj);
	}

	RegisterWeaponEvents(secondaryWeaponObj, true);
}

void ACombatCharacter::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	Super::OnHealthUpdate(InHealthParameters);

	if (!HealthComp->IsAlive())
	{
		TeamFactionComponent->SetCompActive(false);

		if (currentWeaponObj) {
			EndFire();
		}

		if (!InHealthParameters.AffectedHealthComponent->GetIsWounded())
		{
			// check if not using mounted gun as the weapon will be dropped and simulating physics
			if (currentWeaponObj && currentWeaponObj != MountedGun) {
				EndFire();
				currentWeaponObj->DropWeapon(true, true);
			}
		}

		// Unregister the weapon events
		RegisterWeaponEvents(currentWeaponObj, false);
		RegisterWeaponEvents(primaryWeaponObj, false);
		RegisterWeaponEvents(secondaryWeaponObj, false);

		DropMountedGun();

		if (!InHealthParameters.AffectedHealthComponent->GetIsWounded())
		{
			GameModeManager->AddDroppedWeapon(primaryWeaponObj);
			GameModeManager->AddDroppedWeapon(secondaryWeaponObj);
		}
	}
}

void ACombatCharacter::OnWeaponUpdated(FWeaponUpdateParameters WeaponUpdateParameters)
{
	if (WeaponUpdateParameters.WeaponState != EWeaponState::Firing && WeaponUpdateParameters.HasFiredShot) 
	{

		// play voice sound when throwing a grenade.
		if (currentWeaponObj == Cast<AWeapon>(GrenadeWeapon)) {
			PlayVoiceSound(GetVoiceClipsSet()->GrenadeThrowSound);
		}

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

void ACombatCharacter::OnWeaponKillConfirm(FProjectileImpactParameters ProjectileImpactParameters)
{
	EnemyKilled();

	OnKillConfirm.Broadcast(ProjectileImpactParameters.KillCount);
}

// Begin Auto Reload
void ACombatCharacter::OnWeaponAmmoEmpty(AWeapon* Weapon)
{
	// reload if weapon has more ammo
	if (Weapon->getCurrentMaxAmmo() > 0) {
		BeginReload();
		HandGuardAlpha = 0.0f;
	}
	else {

		// if weapon has no more ammo
		if (Weapon->getCurrentMaxAmmo() <= 0) {

			// change current weapon to primary if primary weapon has ammo
			if (primaryWeaponObj->getCurrentMaxAmmo() > 0) {
				EquipWeapon(primaryWeaponObj);
			}
			// otherwise change to secondary if secondary weapon has ammo.
			else if (secondaryWeaponObj->getCurrentMaxAmmo() > 0) {
				EquipWeapon(secondaryWeaponObj);
			}
		}
	}
}

void ACombatCharacter::RegisterWeaponEvents(AWeapon* Weapon, bool BindEvent)
{
	if (Weapon == nullptr) {
		return;
	}

	if (BindEvent)
	{
		Weapon->OnWeaponUpdate.AddDynamic(this, &ACombatCharacter::OnWeaponUpdated);
		Weapon->OnKillConfirmed.AddDynamic(this, &ACombatCharacter::OnWeaponKillConfirm);

		if (CanAutoReloadWeapon)
		{
			Weapon->OnEmptyAmmoClip.AddDynamic(this, &ACombatCharacter::OnWeaponAmmoEmpty);
		}
	}
	else
	{
		Weapon->OnWeaponUpdate.RemoveDynamic(this, &ACombatCharacter::OnWeaponUpdated);
		Weapon->OnKillConfirmed.RemoveDynamic(this, &ACombatCharacter::OnWeaponKillConfirm);
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
		const UEnum* EnumPtr = FindObject<UEnum>(nullptr, TEXT("/Script/FreedomPhantoms.WeaponType"), true);
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
			Loadout->GetMesh()->SetLeaderPoseComponent(GetMesh());
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

void ACombatCharacter::StartCover(FHitResult OutHit, bool IsCrouchOnly)
{
	Super::StartCover(OutHit, IsCrouchOnly);

	EndFire();

	UpdateCombatMode();
}

void ACombatCharacter::StopCover()
{
	Super::StopCover();

	UpdateCombatMode();
}

void ACombatCharacter::UpdateCombatMode()
{
	if (currentWeaponObj == nullptr || !hasEquippedWeapon || IsExitingVehicle) {
		return;
	}

	if (isAiming || isFiring)
	{
		if (!isSprinting && !IsInVehicle && !isTakingCover && !isUsingMountedWeapon)
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

bool ACombatCharacter::CanSwapWeapon()
{
	// if has no weapon.
	if (currentWeaponObj == nullptr || isSwappingWeapon || isEquippingWeapon || isFiring) {
		return false;
	}

	// if currently using primary weapon.
	if (currentWeaponObj == primaryWeaponObj) {
		// can use secondary if exists
		return secondaryWeaponObj != nullptr;
	}
	else {
		// can use primary if exists
		return primaryWeaponObj != nullptr;
	}
}


void ACombatCharacter::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) {
		return;
	}

	if (isReloading) {
		EndReload();
	}

	NewEquippedWeapon = Weapon;

	EndFire();

	// allowing player to aim while reloading in cover has a strange effect.
	if (isTakingCover) {
		EndAim();
	}

	UpdateCombatMode();

	BeginWeaponSwap();
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
		if (currentWeaponObj == Cast<AWeapon>(GrenadeWeapon)) {
			currentWeaponObj->setWeaponSocket(GetMesh(), WeaponHandThrowablesSocket);
		}
		else {
			currentWeaponObj->setWeaponSocket(GetMesh(), WeaponHandSocket);
		}
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

void ACombatCharacter::BeginWeaponSwap()
{
	if (!CanSwapWeapon()) {
		return;
	}

	if (hasEquippedWeapon)
	{
		isSwappingWeapon = true;
		EndFire();
		EndAim();
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
		currentWeaponObj->ReadyToUse();
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

	// if null, then it is assumed there is no specific weapon needed to be equipped.
	if (NewEquippedWeapon == nullptr) {

		// set secondary weapon if current is primary or vice versa
		NewEquippedWeapon = currentWeaponObj == primaryWeaponObj ? secondaryWeaponObj : primaryWeaponObj;
	}

	currentWeaponObj = NewEquippedWeapon;
	currentWeaponObj->ReadyToUse();

	// reset the value to be used again next time otherwise previous if statement will fail if no specific weapon was chosen to be equipped.
	NewEquippedWeapon = nullptr;

	RetrieveWeaponAnimDataSet();
	BeginEquipWeapon();
}

/// <summary>
/// Replace primary & secondary weapons based on weapon type
/// End result will always be one primary as assault or LMG etc. and secondary will be pistol or other handgun types
/// </summary>
/// <param name="Weapon"></param>
void ACombatCharacter::PickupWeapon(AWeapon* NewWeapon)
{
	// Required for the weapon events
	NewWeapon->SetOwner(this);

	// register new weapon events
	RegisterWeaponEvents(NewWeapon, true);

	// update the primary or secondary weapon
	// unregister/ register the weapon events when swapped

	// should the weapon be a primary, if false, then it will be assigned as a secondary weapon.
	bool IsPrimary = false;

	// should the current weapon be dropped after picking up another weapon?
	bool CanDropWeapon = false;

	auto CurrentWeapon = currentWeaponObj;

	// if no current weapon then this pickup weapon will be primary
	if (CurrentWeapon == nullptr) {
		IsPrimary = true;
	}

	// if character is currently using a primary weapon then it should swap current with the pickup weapon.
	// or current weapons is a throwable.
	else if (CurrentWeapon == primaryWeaponObj || CurrentWeapon == Cast<AWeapon>(GrenadeWeapon)) {
		IsPrimary = true;
		CanDropWeapon = true;
		CurrentWeapon = primaryWeaponObj;
	}

	// if there is no secondary weapon, then this pickup weapon will be a secondary
	if (secondaryWeaponObj == nullptr) {
		IsPrimary = false;
		CanDropWeapon = false;
	}

	// if currently using secondary weapon.
	else if (CurrentWeapon == secondaryWeaponObj) {
		IsPrimary = false;
		CanDropWeapon = true;
	}

	if (IsPrimary)
	{
		RegisterWeaponEvents(primaryWeaponObj, false);
		primaryWeaponObj = NewWeapon;
	}
	else
	{
		RegisterWeaponEvents(secondaryWeaponObj, false);
		secondaryWeaponObj = NewWeapon;
	}

	isReloading = false;
	isSwappingWeapon = false;
	EndFire();
	EndAim();
	UpdateCombatMode();

	if (CanDropWeapon) {
		// drop the current weapon
		CurrentWeapon->DropWeapon();

		// set the actor location of current to where the pickup weapon is
		CurrentWeapon->SetActorLocationAndRotation(NewWeapon->GetActorLocation(), NewWeapon->GetActorRotation());

		// assign new weapon to current weapon
		CurrentWeapon = NewWeapon;

		CurrentWeapon->setWeaponSocket(GetMesh(), WeaponHandSocket);

		currentWeaponObj = CurrentWeapon;

		RetrieveWeaponAnimDataSet();
	}
	else {
		Loadout->HolsterWeapon(NewWeapon);
	}
}

void ACombatCharacter::HolsterWeapon()
{
	if (currentWeaponObj == MountedGun) {
		return;
	}

	Loadout->HolsterWeapon(currentWeaponObj);
}


void ACombatCharacter::BeginFire()
{
	if (!CanUseWeapon()) {

		if (isReloading) {
			// Pump Action Weapons can fire if there is ammo
			if (Cast<APumpActionWeapon>(currentWeaponObj) && currentWeaponObj->GetCurrentAmmo() > 0) {
				isReloading = false;
				EndReload();
			}
			else {
				return;
			}
		}

		return;
	}

	// can shoot from cover?
	if (isTakingCover && !isAtCoverCorner && !CanCoverPeakUp()) {
		return;
	}

	// do fire if no throwables available
	if (Cast<AThrowableWeapon>(currentWeaponObj)) {
		if (!currentWeaponObj->CanFireWeapon()) {
			return;
		}
	}

	currentWeaponObj->StartFire();

	isFiring = true;

	UpdateCombatMode();

	if (WeaponAnimDataSet) {
		PlayAnimMontage(WeaponAnimDataSet->Shooting);
	}
}

void ACombatCharacter::EndFire()
{
	if (currentWeaponObj == nullptr || !isFiring) {
		return;
	}

	currentWeaponObj->StopFire();
}


void ACombatCharacter::BeginAim()
{
	if (!CanUseWeapon()) {
		return;
	}

	if (!hasEquippedWeapon) {
		BeginEquipWeapon();
	}
	else {
		Super::BeginAim();
		currentWeaponObj->SetIsAiming(isAiming);
	}

	// if sprinting then stop
	if (isSprinting) {
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

void ACombatCharacter::BeginReload()
{
	if (!CanUseWeapon()) {
		return;
	}

	currentWeaponObj->BeginReload();
	isReloading = currentWeaponObj->getIsReloading();

	if (!isReloading) {
		return;
	}

	EndFire();
	UpdateCombatMode();

	if (WeaponAnimDataSet) {
		PlayAnimMontage(WeaponAnimDataSet->Reloading);
	}

	// reloading grenades shouldn't require to play reload voice sound.
	if (currentWeaponObj != Cast<AWeapon>(GrenadeWeapon) && UKismetMathLibrary::RandomBool()) {
		PlayVoiceSound(GetVoiceClipsSet()->ReloadingSound);
	}
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

bool ACombatCharacter::CanUseWeapon()
{
	return currentWeaponObj && !isReloading && hasEquippedWeapon && !isEquippingWeapon && !isSwappingWeapon;
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


void ACombatCharacter::AimAutoRotation()
{
	if (isTakingCover || !IsInVehicle || !isInCombatMode) {
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

void ACombatCharacter::EnemyKilled()
{
	// don't need to constantly repeat the voice clip after each kill
	if (HasPlayedEnemyKilledSound) {
		return;
	}

	if (UKismetMathLibrary::RandomBool())
	{
		PlayVoiceSound(GetVoiceClipsSet()->EnemyDownSound);
	}

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
		AttachToComponent(MountedGun->GetMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, MountedGun->GetCharacterPositionSocket());
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	// in case character was spawned to use MG
	hasEquippedWeapon = true;

	OnMountedGunEnabled.Broadcast(MountedGun);
}

void ACombatCharacter::DropMountedGun(bool ClearMG)
{
	if (MountedGun == nullptr) {
		return;
	}

	bool CanRemoveOwner = MountedGun->GetOwner() == this;

	// prevent drop animations if not using MG.
	if (isUsingMountedWeapon)
	{
		EndFire();
		EndAim();

		// Unregister the kill event for the MG
		RegisterWeaponEvents(MountedGun, false);

		if (CanRemoveOwner)
		{
			MountedGun->DropWeapon();
		}

		// Reassign to collide with the MG again
		GetCapsuleComponent()->IgnoreActorWhenMoving(MountedGun, false);

		currentWeaponObj = primaryWeaponObj;

		RetrieveWeaponAnimDataSet();
		BeginEquipWeapon();
		GrabWeapon();
	}

	if (ClearMG)
	{
		if (CanRemoveOwner)
		{
			MountedGun->SetOwner(nullptr);
		}

		MountedGun->SetPotentialOwner(nullptr);
		MountedGun = nullptr;
	}

	isUsingMountedWeapon = false;

}

void ACombatCharacter::SetIsExitingVehicle(bool IsExiting)
{
	Super::SetIsExitingVehicle(IsExiting);

	if (IsExiting)
	{
		EndAim();
		EndFire();
		UpdateCombatMode();
	}
}

void ACombatCharacter::CloneCharacter(ACombatCharacter* NewCharacter)
{
	NewCharacter->AddKillCount(KillCounter);
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

void ACombatCharacter::Revived()
{
	Super::Revived();

	BeginEquipWeapon();
}

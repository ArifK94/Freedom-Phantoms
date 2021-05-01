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
#include "Weapons/PumpActionWeapon.h"

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
#include "Kismet/KismetStringLibrary.h"
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

	RetrieveWeaponDataSet();

	if (FactionClass) {
		FactionObj = NewObject<UFactionManager>((UObject*)GetTransientPackage(), FactionClass);
	}

	SpawnHelmet();
	SpawnLoadout();


	if (Loadout)
	{
		primaryWeaponObj = Loadout->SpawnWeapon(WeaponsDataSet, true);
		secondaryWeaponObj = Loadout->SpawnWeapon(WeaponsDataSet, false);
	}

	if (primaryWeaponObj)
	{
		currentWeaponObj = primaryWeaponObj;
	}
	else
	{
		currentWeaponObj = secondaryWeaponObj;
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

void ACombatCharacter::InitTimeHandlers()
{
	Super::InitTimeHandlers();

	GetWorldTimerManager().SetTimer(THandler_CombatMode, this, &ACombatCharacter::UpdateCombatMode, .5f, true);
}

void ACombatCharacter::ClearTimeHandlers()
{
	Super::ClearTimeHandlers();

	GetWorldTimerManager().ClearTimer(THandler_CombatMode);
	GetWorldTimerManager().ClearTimer(THandler_RunAndShoot);
}

void ACombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (isDead) {
		return;
	}

	if (currentWeaponObj)
	{
		UpdatePawnControl();

		UpdateFire();

		UpdateReload();

		if (isEquippingWeapon || isDead)
		{
			EndFire();
		}

		RunAndShoot();
		//disableSprint();
	}
}

void ACombatCharacter::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (isDead)
	{
		if (FactionObj != NULL && GetVoiceClipsSet()->DeathSound != NULL)
		{
			VoiceAudioComponent->Sound = GetVoiceClipsSet()->DeathSound;
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

	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("WeaponType"), true);
	FName WeaponTypeName = EnumPtr->GetNameByValue((int64)currentWeaponObj->GetWeaponType());

	FString Left, Right;
	WeaponTypeName.ToString().Split(TEXT("::"), &Left, &Right);

	static const FString ContextString(TEXT("Weapons Animation DataSet"));
	WeaponAnimDataSet = WeaponsAnimationDatatable->FindRow<FWeaponAnimSet>(*Right, ContextString, true);

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

	//GetWorldTimerManager().SetTimer(THandler_RunAndShoot, this, &ACombatCharacter::RunAndShoot, .3f, true);
}

void ACombatCharacter::EndSprint()
{
	Super::EndSprint();

	GetWorldTimerManager().ClearTimer(THandler_RunAndShoot);
}

void ACombatCharacter::RunAndShoot()
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
		PlayAnimMontage(WeaponAnimDataSet->HolsterMontage);
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
	isEquippingWeapon = true;

	PlayAnimMontage(WeaponAnimDataSet->DrawMontage);
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
		HolsterWeapon();
		hasEquippedWeapon = false;
	}
}

void ACombatCharacter::EndEquipWeapon()
{
	isEquippingWeapon = false;
	isSwappingWeapon = false;

	StopAnimMontage(WeaponAnimDataSet->DrawMontage);
}


void ACombatCharacter::swapWeapon()
{
	if (!isSwappingWeapon) {
		return;
	}

	if (isReloading) {
		EndReload();
	}


	hasEquippedWeapon = false;

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

void ACombatCharacter::HolsterWeapon()
{
	currentWeaponObj->setWeaponSocket(Loadout->GetMesh(), currentWeaponObj->getHolsterSocket());
}


void ACombatCharacter::BeginFire()
{
	if (currentWeaponObj == nullptr || !hasEquippedWeapon || isFiring) {
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
	PlayAnimMontage(WeaponAnimDataSet->Shooting);

}

void ACombatCharacter::EndFire()
{
	if (currentWeaponObj)
	{
		currentWeaponObj->StopFire();
		isFiring = false;
		StopAnimMontage(WeaponAnimDataSet->Shooting);
	}
}

void ACombatCharacter::UpdateCombatMode()
{
	if (currentWeaponObj && hasEquippedWeapon && !isReloading && !isRepellingDown)
	{
		if (isAiming || isFiring)
		{
			isInCombatMode = true;
			SetHandGaurdIK(1.0f);
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
		HandGuardAlpha = 0.0f;
	}

	if (CanAutoReloadWeapon && currentWeaponObj->getCurrentAmmo() <= 0) {
		BeginReload();
		HandGuardAlpha = 0.0f;
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
			Super::BeginAim();
			currentWeaponObj->SetIsAiming(true);
		}

		EndSprint();
	}
}

void ACombatCharacter::EndAim()
{
	Super::EndAim();

	if (currentWeaponObj) {
		currentWeaponObj->SetIsAiming(false);
	}
}

void ACombatCharacter::BeginReload()
{
	if (currentWeaponObj == nullptr || isReloading) {
		return;
	}

	if (currentWeaponObj->getCurrentMaxAmmo() <= 0 || currentWeaponObj->getCurrentAmmo() >= currentWeaponObj->getAmmoPerClip()) {
		return;
	}

	currentWeaponObj->BeginReload();
	isAiming = false;

	PlayAnimMontage(WeaponAnimDataSet->Reloading);

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
		StopAnimMontage(WeaponAnimDataSet->Reloading);
	}
}

void ACombatCharacter::UpdateReload()
{
	if (isDead || currentWeaponObj == nullptr) {
		return;
	}

	isReloading = currentWeaponObj->getIsReloading();
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
		if (FactionObj != NULL && GetVoiceClipsSet()->TargetFoundSound != NULL)
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

	if (FactionObj != NULL && GetVoiceClipsSet()->FriendlyDownSound != NULL)
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
		if (FactionObj != NULL && GetVoiceClipsSet()->EnemyDownSound != NULL)
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

void ACombatCharacter::UseMountedGun(AWeapon* MountedGun)
{
	isUsingMountedWeapon = true;
	HolsterWeapon();
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


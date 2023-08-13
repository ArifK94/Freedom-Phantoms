// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Interfaces/Interactable.h"
#include "CombatCharacter.generated.h"

class AWeapon;
class AMountedGun;
class ACommanderCharacter;
class UTeamFactionComponent;
class AThrowableWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatUpdatedignature, ACombatCharacter*, CombatCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillConfirmSignature, int, KillCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMountedGunEnabledSignature, AMountedGun*, MountedGun);

UCLASS()
class FREEDOMPHANTOMS_API ACombatCharacter : public ABaseCharacter, public IInteractable
{
	GENERATED_BODY()

public:
#pragma region DataTables
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		UDataTable* WeaponsDatatable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		FName WeaponsRowName;
	FWeaponsSet* WeaponsDataSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		UDataTable* WeaponsAnimationDatatable;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FWeaponAnimSet WeaponAnimDataSetEditor;
	FWeaponAnimSet* WeaponAnimDataSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		UDataTable* FactionDatatable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		FName FactionRowName;
	FFaction* FactionDataSet;

	
	FTimerHandle THandler_Datatable;

private:
	void RetrieveWeaponDataSet();

	void RetrieveWeaponAnimDataSet();

	void RetrieveFactionDataSet();

#pragma endregion

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTeamFactionComponent* TeamFactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isFiring;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isReloading;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isEquippingWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool isInCombatMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isSwappingWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool hasEquippedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool CanAutoReloadWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isUsingMountedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName WeaponHandSocket;

	/** Socket for the grenades & other throwables */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName WeaponHandThrowablesSocket;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessory", meta = (AllowPrivateAccess = "true"))
		AHeadgear* Headgear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessory", meta = (AllowPrivateAccess = "true"))
		ALoadout* Loadout;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Faction Manager", meta = (AllowPrivateAccess = "true"))
		ACommanderCharacter* CommandingOfficer;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		AWeapon* currentWeaponObj;

	/** Attach if specific weapon is to be set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeapon> PrimaryWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		AWeapon* primaryWeaponObj;

	/** Attach if specific weapon is to be set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeapon> SecondaryWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		AWeapon* secondaryWeaponObj;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		AThrowableWeapon* GrenadeWeapon;

	UPROPERTY()
		AMountedGun* MountedGun;

	/** Used for equipping specific weapons. */
	UPROPERTY()
		AWeapon* NewEquippedWeapon;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		float MaxAimYawSprint;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		float HandGuardAlpha;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		int KillCounter;

	bool IsInAimOffSetRotation;

	bool HasPlayedEnemyKilledSound;

	FTimerHandle THandler_VoiceSoundReset;

public:
	ACombatCharacter();

	virtual void SetDefaultState() override;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnCombatUpdatedignature OnCombatUpdated;

	/**
	* To be triggered for commander to recieve when an operative gets a kill
	*/
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnKillConfirmSignature OnKillConfirm;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnMountedGunEnabledSignature OnMountedGunEnabled;

	UFUNCTION(BlueprintCallable)
		void SetPrimaryWeapon(AWeapon* Weapon);

	UFUNCTION(BlueprintCallable)
		void SetSecondaryWeapon(AWeapon* Weapon);

	virtual void BeginSprint() override;
	virtual void EndSprint() override;

	virtual	void UpdateCharacterMovement() override;

	virtual	void BeginAim() override;
	virtual	void EndAim() override;

	virtual void StopCover() override;


	UFUNCTION(BlueprintCallable, Category = "Weapon Actions")
		void BeginFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon Actions")
		void EndFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon Actions")
		void BeginReload();
	void EndReload();


	UFUNCTION(BlueprintCallable)
		void OnWeaponAmmoEmpty(AWeapon* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Weapon Actions")
		void ToggleNightVision();

	UFUNCTION(BlueprintCallable, Category = "Weapon Actions")
		void BeginWeaponSwap();

	UFUNCTION()
		void OnWeaponUpdated(FWeaponUpdateParameters WeaponUpdateParameters);

	UFUNCTION()
		void OnWeaponKillConfirm(FProjectileImpactParameters ProjectileImpactParameters);
	void RegisterWeaponEvents(AWeapon* Weapon, bool BindEvent);

	/** In case a secondary weapon is not present or any weapons at all. */
	bool CanSwapWeapon();

	UFUNCTION(BlueprintCallable)
		void BeginEquipWeapon();

	UFUNCTION(BlueprintCallable)
		void GrabWeapon();

	UFUNCTION(BlueprintCallable)
		void EndEquipWeapon();


	UFUNCTION(BlueprintCallable)
		void EquipWeapon(AWeapon * Weapon);

	void swapWeapon();

	void PickupWeapon(AWeapon* NewWeapon);

	void HolsterWeapon();

	void AimAutoRotation();

	void DisableSprint();

	void SetHandGaurdIK(float Alpha);

	void EnemyKilled();

	void ResetVoiceSound();

	void SetMountedGun(AWeapon* Mount);

	void UseMountedGun();

	void DropMountedGun(bool ClearMG = true);

	virtual void SetIsExitingVehicle(bool IsExiting) override;

	void CloneCharacter(ACombatCharacter* NewCharacter);

private:
	void SpawnHelmet();
	void SpawnLoadout(LoadoutType LoadoutType = LoadoutType::Assault, bool SpecifyType = false);

	void UpdateCombatMode();

	virtual void StartCover(FHitResult OutHit, bool IsCrouchOnly) override;

	/**
	* Can fire, reload etc. current weapon.
	*/
	bool CanUseWeapon();

protected:
	virtual void Init() override;
	virtual void BeginPlay() override;

	virtual void OnHealthUpdate(FHealthParameters InHealthParameters) override;
	virtual void PlayDeathAnim(FHealthParameters InHealthParameters) override;

	virtual void Revived() override;

public:
	// Interactable interface methods
	virtual FString GetKeyDisplayName_Implementation() override;
	virtual FString OnInteractionFound_Implementation(APawn* InPawn, AController* InController) override;
	virtual AActor* OnPickup_Implementation(APawn* InPawn, AController* InController) override;
	virtual bool OnUseInteraction_Implementation(APawn* InPawn, AController* InController) override;
	virtual bool CanInteract_Implementation(APawn* InPawn, AController* InController) override;

public:
	UTeamFactionComponent* GetTeamFactionComponent() { return TeamFactionComponent; }

	bool IsFiring() {
		return isFiring;
	}

	bool IsUsingMountedWeapon() {
		return isUsingMountedWeapon;
	}
	
	bool IsReloading() {
		return isReloading;
	}

	bool IsSwappingWeapon() {
		return isSwappingWeapon;
	}

	AWeapon* GetCurrentWeapon() {
		return currentWeaponObj;
	}

	AWeapon* GetPrimaryWeapon() {
		return primaryWeaponObj;
	}

	AWeapon* GetSecondaryWeaponObj(){
		return secondaryWeaponObj;
	}

	AThrowableWeapon* GetGrenadeWeapon() { return GrenadeWeapon; }

	AMountedGun* GetMountedGun() {
		return MountedGun;
	}

	bool IsInCombatMode() {
		return isInCombatMode;
	}

	void IsInCombatMode(bool Value) {
		isInCombatMode = Value;
	}

	FName GetFactionRowName() {
		return FactionRowName;
	}

	FFaction* GetFactionDataSet() {
		return FactionDataSet;
	}


	void setCommandingOfficer(ACommanderCharacter* Commander) {
		CommandingOfficer = Commander;
	}

	ACommanderCharacter* GetCommander() {
		return CommandingOfficer;
	}

	void AddKillCount(int Amount) {
		KillCounter += Amount;
	}
};

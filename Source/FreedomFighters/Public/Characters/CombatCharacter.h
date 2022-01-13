// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "CombatCharacter.generated.h"

class AWeapon;
class AMountedGun;
class ACommanderCharacter;
class UTeamFactionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatUpdatedignature, ACombatCharacter*, CombatCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillConfirmSignature, int, KillCount);

UCLASS()
class FREEDOMFIGHTERS_API ACombatCharacter : public ABaseCharacter
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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessory", meta = (AllowPrivateAccess = "true"))
		AHeadgear* Headgear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessory", meta = (AllowPrivateAccess = "true"))
		ALoadout* Loadout;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Faction Manager", meta = (AllowPrivateAccess = "true"))
		ACommanderCharacter* CommandingOfficer;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AWeapon* currentWeaponObj;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AWeapon* primaryWeaponObj;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AWeapon* secondaryWeaponObj;

	AMountedGun* MountedGun;


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

	FOnCombatUpdatedignature OnCombatUpdated;
	FOnKillConfirmSignature OnKillConfirm; // to be triggered for commander to recieve when an operative gets a kill

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
		void OnWeaponKillConfirm(int KillCount, bool IsSingleKill, bool IsDoubleKill, bool IsMultiKill);
	void RegisterWeaponEvents(AWeapon* Weapon, bool BindEvent);

	void BeginEquipWeapon();
	void GrabWeapon();
	void EndEquipWeapon();

	void swapWeapon();

	void PickupWeapon(AWeapon* Weapon);

	void HolsterWeapon();

	void AimAutoRotation();

	void DisableSprint();

	void SetHandGaurdIK(float Alpha);

	ACombatCharacter* FindNearestFriendly();

	void FriendlyKilled();

	void EnemyKilled();

	void ResetVoiceSound();

	void ShowCharacterOutline(bool CanShow, bool IgnoreDeath = false) override;

	void SetMountedGun(AWeapon* Mount);

	void UseMountedGun();

	void DropMountedGun(bool ClearMG = true);

	virtual void SetIsRepellingDown(bool IsRappelling) override;

private:
	void SpawnHelmet();
	void SpawnLoadout(LoadoutType LoadoutType = LoadoutType::Assault, bool SpecifyType = false);

	void UpdateCombatMode();

protected:
	virtual void BeginPlay() override;

	virtual void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo) override;
	virtual void PlayDeathAnim(AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo) override;

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

	AMountedGun* GetMountedGun() {
		return MountedGun;
	}

	bool IsInCombatMode() {
		return isInCombatMode;
	}

	void IsInCombatMode(bool Value) {
		isInCombatMode = Value;
	}

	FFaction* GetFactionDataSet() {
		return FactionDataSet;
	}


	void setCommandingOfficer(ACommanderCharacter* Commander) {
		CommandingOfficer = Commander;
	}

	ACommanderCharacter* getCommander() {
		return CommandingOfficer;
	}


	void SetPrimaryWeapon(AWeapon* Weapon);
	void SetSecondaryWeapon(AWeapon* Weapon);

	void SetKillCount(int Amount) {
		KillCounter += Amount;
	}
};

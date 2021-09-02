// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "CombatCharacter.generated.h"

class AWeapon;
class AMountedGun;
class ACommanderCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatUpdatedignature, ACombatCharacter*, CombatCharacter);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		AWeapon* underBarrelWeaponObj;
	AMountedGun* MountedGun;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		float MaxAimYawSprint;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		float HandGuardAlpha;


	bool IsInAimOffSetRotation;

	bool HasPlayedTargetFoundSound;
	bool HasPlayedEnemyKilledSound;

	FTimerHandle THandler_VoiceSoundReset;

public:
	ACombatCharacter();

	FOnCombatUpdatedignature OnCombatUpdated;

	virtual void BeginSprint() override;
	virtual void EndSprint() override;



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

	void BeginEquipWeapon();
	void GrabWeapon();
	void EndEquipWeapon();

	void swapWeapon();

	void HolsterWeapon();

	void AimAutoRotation();

	void DisableSprint();

	void ToggleUnderBarrelWeapon();

	void SetHandGaurdIK(float Alpha);

	void ToggleLaser();

	void ToggleLight();

	UFUNCTION(BlueprintCallable, Category = "Combat Actions")
		void TargetFound();

	UFUNCTION(BlueprintCallable, Category = "Combat Actions")
		ACombatCharacter* FindNearestFriendly();

	UFUNCTION(BlueprintCallable, Category = "Combat Actions")
		ACombatCharacter* FindNearestEnemy(float TargetRange);


	void FriendlyKilled();

	void EnemyKilled();

	void ResetVoiceSound();

	void ShowCharacterOutline(bool CanShow) override;

	void SetMountedGun(AWeapon* Mount);

	void UseMountedGun();

	void DropMountedGun(bool ClearMG = true);

	virtual void SetIsRepellingDown(bool IsRappelling) override;


private:

	void SpawnHelmet();
	void SpawnLoadout();

	void UpdateCombatMode();

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	virtual void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo) override;
	virtual void PlayDeathAnim(AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo) override;

public:
	bool IsFiring() {
		return isFiring;
	}

	bool IsUsingMountedWeapon() {
		return isUsingMountedWeapon;
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

	bool IsReloading() {
		return isReloading;
	}

	AWeapon* GetCurrentWeapon() {
		return currentWeaponObj;
	}

	AWeapon* GetPrimaryWeapon() {
		return primaryWeaponObj;
	}

	AWeapon* GetUnderBarrelWeapon() {
		return underBarrelWeaponObj;
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


	void SetPrimaryWeapon(AWeapon* Weapon);

	void SetSecondaryWeapon(AWeapon* Weapon);

};

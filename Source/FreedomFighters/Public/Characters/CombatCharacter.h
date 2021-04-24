// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "CombatCharacter.generated.h"

class AWeapon;
class UFactionManager;
class ACommanderCharacter;
UCLASS()
class FREEDOMFIGHTERS_API ACombatCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isFiring;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isReloading;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isEquippingWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isInCombatMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isSwappingWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool hasEquippedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool CanAutoReloadWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isUsingMountedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName WeaponHandSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName SecondaryWeaponHandSocket;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Faction Manager", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UFactionManager> FactionClass;

	UFactionManager* FactionObj;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Faction Manager", meta = (AllowPrivateAccess = "true"))
		ACommanderCharacter* CommandingOfficer;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AWeapon* currentWeaponObj;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AWeapon* primaryWeaponObj;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AWeapon* secondaryWeaponObj;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AWeapon* underBarrelWeaponObj;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		float MaxAimYawSprint;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		float HandGuardAlpha;


	bool IsInAimOffSetRotation;

	bool HasPlayedTargetFoundSound;
	bool HasPlayedEnemyKilledSound;

	FTimerHandle THandler_HandguardIK;
	FTimerHandle THandler_CombatMode;
	FTimerHandle THandler_FireWeapon;
	FTimerHandle THandler_RunAndShoot;

	FTimerHandle THandler_VoiceSoundReset;

public:
	ACombatCharacter();

	void UpdateCombatMode();

	UFUNCTION(BlueprintCallable, Category = "Weapon Actions")
		void BeginFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon Actions")
		void EndFire();
	void UpdateFire();

	virtual	void BeginAim() override;
	virtual	void EndAim() override;

	UFUNCTION(BlueprintCallable, Category = "Weapon Actions")
		void BeginReload();
	void EndReload();
	void UpdateReload();

	UFUNCTION(BlueprintCallable, Category = "Weapon Actions")
		void ToggleNightVision();

	void UpdatePawnControl();

	UFUNCTION(BlueprintCallable, Category = "Weapon Actions")
		void BeginWeaponSwap();

	void BeginEquipWeapon();
	void GrabWeapon();
	void EndEquipWeapon();

	void swapWeapon();

	void HolsterWeapon();

	void setWeaponHand();

	void DropMountedGun();

	void RunAndShoot ();

	void disableSprint();

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

	void UseMountedGun(AWeapon* MountedGun);

private:
	AHeadgear* Headgear;
	ALoadout* Loadout;

	void SpawnHelmet();
	void SpawnLoadout();


protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) override;

public:
	bool IsFiring() {
		return isFiring;
	}

	bool IsUsingMountedWeapon() {
		return isUsingMountedWeapon;
	}

	UFactionManager* getFactionObj() {
		return FactionObj;
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

	void IsInCombatMode(bool Value) {
		isInCombatMode = Value;
	}

};

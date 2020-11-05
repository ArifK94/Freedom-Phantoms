// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "CombatCharacter.generated.h"

/**
 *
 */
UCLASS()
class FREEDOMFIGHTERS_API ACombatCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ACombatCharacter();

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName WeaponHandSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName SecondaryWeaponHandSocket;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Faction Manager", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class UFactionManager> FactionClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Faction Manager", meta = (AllowPrivateAccess = "true"))
		UFactionManager* FactionObj;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Faction Manager", meta = (AllowPrivateAccess = "true"))
		class ACommanderCharacter* CommandingOfficer;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AWeapon* currentWeaponObj;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AWeapon* primaryWeaponObj;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AWeapon* secondaryWeaponObj;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AWeapon* underBarrelWeaponObj;



	class ALoadout* loadoutObj;

	class AHeadgear* headgearObj;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		float MaxAimYawSprint;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		float HandGuardAlpha;


	bool IsInAimOffSetRotation;

	bool HasPlayedReloadingSound;
	bool HasPlayedTargetFoundSound;
	bool HasPlayedEnemyKilledSound;


	FTimerHandle THandler_VoiceSoundReset;


public:

	AWeapon* GetCurrentWeapon();

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

	void unEquipWeapon();

	void setWeaponHand();

	void setCharacterRotation();

	void disableSprint();

	void ToggleUnderBarrelWeapon();

	void UpdateHandGaurdIK();

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

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
	UFactionManager* getFactionObj()
	{
		return FactionObj;
	}


	void setCommandingOfficer(ACommanderCharacter* Commander) { CommandingOfficer = Commander; }

	ACommanderCharacter* getCommander() {
		return CommandingOfficer;
	}

	bool IsReloading() {
		return isReloading;
	}

};

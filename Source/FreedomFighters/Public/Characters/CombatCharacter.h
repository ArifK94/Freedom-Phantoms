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
		bool isAiming;

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

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AWeapon* currentWeaponObj;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AWeapon* primaryWeaponObj;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AWeapon* secondaryWeaponObj;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AWeapon* underBarrelWeaponObj;


	UPROPERTY(EditAnywhere, Category = "Accessories")
		TArray<TSubclassOf<class ALoadout>> Loadouts;
	class ALoadout* loadoutObj;

	UPROPERTY(EditAnywhere, Category = "Accessories")
		TArray<TSubclassOf<class AHeadgear>> Headgears;
	class AHeadgear* headgearObj;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		float MaxAimYawSprint;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float HandGuardAlpha;


	bool IsInAimOffSetRotation;

public:

	AWeapon* GetCurrentWeapon();

	void UpdateCombatMode();

	void BeginFire();
	void EndFire();
	void UpdateFire();

	void BeginAim();
	void EndAim();

	void BeginReload();
	void EndReload();
	void UpdateReload();

	void ToggleNightVision();

	void UpdatePawnControl();

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


private:

	void SpawnHelmet();
	void SpawnLoadout();

	virtual void UpdateSprint() override;


protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	
};

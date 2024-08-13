// Fill out your copyright notice in the Description page of Project Settings.

/**
 * Grenades, Molotovs etc.
 */

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "ThrowableWeapon.generated.h"

UCLASS()
class FREEDOMPHANTOMS_API AThrowableWeapon : public AWeapon
{
	GENERATED_BODY()

private:
	/** The angle when throwing the projectile. used for AI to directly throw at the target */
	FRotator VolleyAngle;

public:
	AThrowableWeapon();

	virtual FString OnInteractionFound_Implementation(APawn* InPawn, AController* InController) override;
	virtual AActor* OnPickup_Implementation(APawn* InPawn, AController* InController) override;

	virtual void SetIsAiming(bool isAiming) override;

	virtual void Fire() override;

	virtual void StartFire() override;

	virtual void CreateBullet() override;

	virtual void setWeaponSocket(USkeletalMeshComponent* meshComponent, FName socket) override;

	virtual void HolsterWeapon(USkeletalMeshComponent* Parent) override;

	virtual void DropWeapon(bool RemoveOwner = true, bool SimulatePhysics = false) override;

protected:
	virtual void OnReload() override;


public:
	void SetVolleyAngle(FRotator Angle) { VolleyAngle = Angle; }

};

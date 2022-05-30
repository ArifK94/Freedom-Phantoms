// Fill out your copyright notice in the Description page of Project Settings.

/**
 * Grenades, Molotovs etc.
 */

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "ThrowableWeapon.generated.h"

UCLASS()
class FREEDOMFIGHTERS_API AThrowableWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	AThrowableWeapon();

	virtual FString OnInteractionFound_Implementation(APawn* InPawn, AController* InController) override;
	virtual AActor* OnPickup_Implementation(APawn* InPawn, AController* InController) override;

	virtual void SetIsAiming(bool isAiming) override;

	virtual void Fire() override;

	virtual void StartFire() override;

};

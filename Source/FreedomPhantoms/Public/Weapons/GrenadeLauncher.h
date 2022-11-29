// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "GrenadeLauncher.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMPHANTOMS_API AGrenadeLauncher : public AWeapon
{
	GENERATED_BODY()
	
public:
	AGrenadeLauncher();

private:
	virtual void Fire() override;

};

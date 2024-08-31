// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShootPauseSignature, float, ResumeDelay);

class AWeapon;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FREEDOMPHANTOMS_API UShooterComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	FTimerHandle THandler_BeginShoot;
	FTimerHandle THandler_ResumeShoot;

	UPROPERTY()
	TArray<AWeapon*> Weapons;

	/**
	* Select a random weapon to fire from instead of firing all weapons.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool FireRandomWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float m_TimeBetweenShotsMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float m_TimeBetweenShotsMax;

	bool HasFired;


public:
	UShooterComponent();

	UFUNCTION(BlueprintCallable)
	void BeginFire();

	UFUNCTION(BlueprintCallable)
	void PauseFire();

	UFUNCTION(BlueprintCallable)
	void EndFire();

	UFUNCTION(BlueprintCallable)
	void SetWeapons(TArray<AWeapon*> InWeapon);

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnShootPauseSignature OnShootPause;

public:
	float TimeBetweenShotsMin() { return m_TimeBetweenShotsMin; }
	float TimeBetweenShotsMax() { return m_TimeBetweenShotsMax; }

	void SetTimeBetweenShotsMin(float Value) { m_TimeBetweenShotsMin = Value; }
	void SetTimeBetweenShotsMax(float Value) { m_TimeBetweenShotsMax = Value; }

	TArray<AWeapon*> GetWeapons() { return Weapons; }

	void AddWeapon(AWeapon* Weapon);

};

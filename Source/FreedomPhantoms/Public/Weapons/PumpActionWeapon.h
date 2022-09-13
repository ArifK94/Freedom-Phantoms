// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "PumpActionWeapon.generated.h"

/**
 *
 */
UCLASS()
class FREEDOMPHANTOMS_API APumpActionWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	APumpActionWeapon();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* PumpPullSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* PumpPushSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* InsertAmmoSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool HasLoadedShell;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsPullingPump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool HasFinishedReload;

	/** An animation may not be present for when reloading a shell so using alternative to pump shell like pull and push sounds should work */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool PumpActionBySound;

	FTimerHandle THandler_Pump;

public:

	void PlayPumpPullSound();

	void PlayPumpPushSound();

	void BeginLoadShell();

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
		void EndLoadShell();

private:

	virtual void BeginPlay() override;

	virtual void Fire() override;


	virtual void OnReload() override;



public:

	bool GetHasLoadedShell() {
		return HasLoadedShell;
	}

};

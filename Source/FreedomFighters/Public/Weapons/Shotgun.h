// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMFIGHTERS_API AShotgun : public AWeapon
{
	GENERATED_BODY()

public:
	AShotgun();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Sounds", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* PumpAudioComponent;

	FTimerHandle THandler_PullPump;


private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* HandguardPushSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* HandguardPullSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* InsertAmmoSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool HasLoadedShell;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool IsPullingPump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool HasFinishedReload;

	/** An animation may not be present for when reloading a shell so using alternative to pump shell like pull and push sounds should work */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool PumpActionBySound;


public:

	void PlayHandguardPullSound();

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
		void PlayHandguardPushSound();

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
		void PullPump();

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
		void PushPump();

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
		void BeginHandguardTransition();

	void beginHandguardPull();


private:

	virtual void BeginPlay() override;

	virtual void Fire() override;


	virtual void OnReload() override;



public:

	bool GetHasLoadedShell() {
		return HasLoadedShell;
	}

	
};

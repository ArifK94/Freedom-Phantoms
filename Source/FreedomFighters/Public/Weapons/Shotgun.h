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



	UStaticMeshComponent* HandguardComp;
	UAudioComponent* HandguardAudioComponent;



	FTimerHandle pullHandguardTimeHandle;

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Ammo_Holder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* HandguardPushSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* HandguardPullSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* InsertAmmoSound;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handguard", meta = (AllowPrivateAccess = "true"))
		float HandguardSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool hasLoadedShell;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool isPullingHandguard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool hasFinishedReload;



public:

	void PlayHandguardPullSound();

	UFUNCTION(BlueprintCallable, Category = "Shotgun", meta = (AllowPrivateAccess = "true"))
		void PlayHandguardPushSound();

	UFUNCTION(BlueprintCallable, Category = "Shotgun", meta = (AllowPrivateAccess = "true"))
		void pullHanguard();

	UFUNCTION(BlueprintCallable, Category = "Shotgun", meta = (AllowPrivateAccess = "true"))
		void pushHanguard();

	UFUNCTION(BlueprintCallable, Category = "Shotgun", meta = (AllowPrivateAccess = "true"))
		void BeginHandguardTransition();

	void beginHandguardPull();


private:

	virtual void BeginPlay() override;

	void Tick(float DeltaTime);



	virtual void Fire() override;

	virtual void OnReload() override;

	void setHandguard();

	
};

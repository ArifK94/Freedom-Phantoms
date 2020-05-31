// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "Weapons/AssaultRifle.h"


#include "GameManager.generated.h"

class UParticleSystem;
class UCameraShake;
UCLASS()
class FREEDOMFIGHTERS_API AGameManager : public AGameMode
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Effects")
		UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Effects")
		UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Effects")
		UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Effects")
		UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Effects")
		TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(EditAnywhere, Category = "Headgears")
		TArray<TSubclassOf<class AHeadgear>> headgears;
	class AHeadgear* headgearObj;

	// Load-out
	UPROPERTY(EditAnywhere, Category = "Loadouts")
		TArray<TSubclassOf<class ALoadout>> loadouts;
	class ALoadout* loadoutObj;

	class AWeapon* weaponPrimaryObj;
	class AWeapon* weaponSecondaryObj;



	UPROPERTY(EditAnywhere, Category = "Assault Weapons")
		TArray<TSubclassOf<class AAssaultRifle>> AssaultRifles;

	UPROPERTY(EditAnywhere, Category = "SMG Weapons")
		TArray<TSubclassOf<class ASMG>> SMGs;

	UPROPERTY(EditAnywhere, Category = "Shotgun Weapons")
		TArray<TSubclassOf<class AShotgun>> Shotguns;

	UPROPERTY(EditAnywhere, Category = "LMG Weapons")
		TArray<TSubclassOf<class ALMG>> LMGs;


	UPROPERTY(EditAnywhere, Category = "Secondary Weapons")
		TArray<TSubclassOf<class AWeapon>> SecondaryWeapons;
	UPROPERTY(EditAnywhere, Category = "Secondary Weapons")
		TArray<TSubclassOf<class AWeapon>> Pistols;


	UPROPERTY(EditAnywhere, Category = "Audio Battle Chatters")
		TArray<TSubclassOf<class USoundBase>> ReloadingClips;


private:
	FVector holsterPrimaryLocation;
	FRotator holsterPrimaryRotation;

	FVector holsterSideArmLocation;
	FRotator holsterSideArmRotation;

public:


	void SpawnHelmet(USkeletalMeshComponent* mesh, AActor* owner);
	ALoadout* SpawnLoadout(USkeletalMeshComponent* mesh, AActor* owner);
	AWeapon* SpawnWeapon(USkeletalMeshComponent* mesh, AActor* owner, TArray<TSubclassOf<class AWeapon>> weapons);

	AWeapon* SpawnAssaultRifle(USkeletalMeshComponent* mesh, AActor* owner);
	AWeapon* SpawnSMG(USkeletalMeshComponent* mesh, AActor* owner);
	AWeapon* SpawnShotgun(USkeletalMeshComponent* mesh, AActor* owner);
	AWeapon* SpawnLMG(USkeletalMeshComponent* mesh, AActor* owner);

	AWeapon* SpawnSecondaryWeapon(USkeletalMeshComponent* mesh, AActor* owner);
	AWeapon* SpawnPistol(USkeletalMeshComponent* mesh, AActor* owner);


	UParticleSystem* CheckSurface(EPhysicalSurface SurfaceType);


	void setHolsterPrimaryLocation(FVector location);
	void setHolsterPrimaryRotation(FRotator rotation);

	void setHolsterSideArmLocation(FVector location);
	void setHolsterSideArmRotation(FRotator rotation);
	
};

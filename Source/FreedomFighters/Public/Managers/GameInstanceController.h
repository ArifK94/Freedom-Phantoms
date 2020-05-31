// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameInstanceController.generated.h"

class UParticleSystem;
class UCameraShake;

UCLASS()
class FREEDOMFIGHTERS_API UGameInstanceController : public UGameInstance
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

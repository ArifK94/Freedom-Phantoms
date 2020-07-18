// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponSet.generated.h"


class USkeletalMeshComponent;
class AWeapon;

UCLASS(Blueprintable)
class FREEDOMFIGHTERS_API UWeaponSet : public UObject
{
	GENERATED_BODY()

protected:
	virtual bool IsSupportedForNetworking() const override { return true; };

public:

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


public:

	UWeaponSet();

	AWeapon* SpawnAssaultRifle(UWorld* world, USkeletalMeshComponent* mesh, AActor* owner);
	AWeapon* SpawnSMG(UWorld* world, USkeletalMeshComponent* mesh, AActor* owner);
	AWeapon* SpawnShotgun(UWorld* world, USkeletalMeshComponent* mesh, AActor* owner);
	AWeapon* SpawnLMG(UWorld* world, USkeletalMeshComponent* mesh, AActor* owner);

	AWeapon* SpawnSecondaryWeapon(UWorld* world, USkeletalMeshComponent* mesh, AActor* owner);
	AWeapon* SpawnPistol(UWorld* world, USkeletalMeshComponent* mesh, AActor* owner);
	
};

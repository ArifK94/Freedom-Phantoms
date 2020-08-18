// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FactionManager.generated.h"

class AWeapon;
class UWeaponSet;
class USoundBase;
class AHeadgear;
class ALoadout;
UCLASS(Blueprintable)
class FREEDOMFIGHTERS_API UFactionManager : public UObject
{
	GENERATED_BODY()

protected:
	virtual bool IsSupportedForNetworking() const override { return true; };


private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attachment", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UWeaponSet> WeaponSetClass;
	UWeaponSet* WeaponSetObj;


	UPROPERTY(EditAnywhere, Category = "Accessories")
		TArray<TSubclassOf< ALoadout>> Loadouts;
	ALoadout* loadoutObj;

	UPROPERTY(EditAnywhere, Category = "Accessories")
		TArray<TSubclassOf< AHeadgear>> Headgears;
	AHeadgear* headgearObj;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Battle Chatters", meta = (AllowPrivateAccess = "true"))
		TArray<USoundBase*> TargetFoundClips;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Battle Chatters", meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<USoundBase>> ReloadingClips;

private:
	UWorld* CurrentWorld;

	AWeapon* PrimaryWeaponObj;
	AWeapon* SecondaryWeaponObj;

public:
	UFactionManager();
	void Init(UWorld* World);

public:

	UWeaponSet* getWeaponSetObj() { return WeaponSetObj; }

	AWeapon* getPrimaryWeaponObj() { return PrimaryWeaponObj; }
	AWeapon* getSecondaryWeaponObj() { return SecondaryWeaponObj; }

	AHeadgear* SpawnHelmet(USkeletalMeshComponent* Mesh, AActor* Owner);
	ALoadout* SpawnLoadout(USkeletalMeshComponent* Mesh, AActor* Owner);
};

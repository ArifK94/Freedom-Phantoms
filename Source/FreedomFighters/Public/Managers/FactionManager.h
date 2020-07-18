// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FactionManager.generated.h"

class AWeapon;
UCLASS(Blueprintable)
class FREEDOMFIGHTERS_API UFactionManager : public UObject
{
	GENERATED_BODY()

protected:
	virtual bool IsSupportedForNetworking() const override { return true; };


public:

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attachment", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class UWeaponSet> WeaponSetClass;
	UWeaponSet* WeaponSetObj;


	UPROPERTY(EditAnywhere, Category = "Accessories")
		TArray<TSubclassOf<class ALoadout>> Loadouts;
	class ALoadout* loadoutObj;

	UPROPERTY(EditAnywhere, Category = "Accessories")
		TArray<TSubclassOf<class AHeadgear>> Headgears;
	class AHeadgear* headgearObj;

private:
	UWorld* WorldOwner;

	AWeapon* PrimaryWeaponObj;
	AWeapon* SecondaryWeaponObj;

public:
	void Init(UWorld* World);
	void SetWeaponTypeClass(USkeletalMeshComponent* mesh, AActor* owner);

public:

	AWeapon* getPrimaryWeaponObj() { return PrimaryWeaponObj; }
	AWeapon* getSecondaryWeaponObj() { return SecondaryWeaponObj; }

private:
	void SpawnHelmet();
	void SpawnLoadout();
};

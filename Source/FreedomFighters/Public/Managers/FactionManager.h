// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"

#include "FactionManager.generated.h"

class AWeapon;
class UWeaponSet;
class USoundBase;
class AHeadgear;
class ALoadout;



USTRUCT(BlueprintType)
struct FVoiceClipSet : public FTableRowBase
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* TargetFoundSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* ReloadingSound;
};



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
		TArray<FVoiceClipSet>	VoiceClips;
	FVoiceClipSet SelectedVoiceClipSet;

private:
	UWorld* CurrentWorld;

	AWeapon* PrimaryWeaponObj;
	AWeapon* SecondaryWeaponObj;

public:
	UFactionManager();
	void Init(UWorld* World);

public:

	void setRanomVoiceSet();

	FVoiceClipSet getSelectedVoiceClipSet() { return SelectedVoiceClipSet; }

	UWeaponSet* getWeaponSetObj() { return WeaponSetObj; }

	AWeapon* getPrimaryWeaponObj() { return PrimaryWeaponObj; }
	AWeapon* getSecondaryWeaponObj() { return SecondaryWeaponObj; }

	AHeadgear* SpawnHelmet(USkeletalMeshComponent* Mesh, AActor* Owner);
	ALoadout* SpawnLoadout(USkeletalMeshComponent* Mesh, AActor* Owner);
};

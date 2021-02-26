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
class AAircraft;
class ACombatCharacter;
class ACommanderCharacter;

USTRUCT(BlueprintType)
struct FVoiceClipSet : public FTableRowBase
{
	GENERATED_BODY()
		
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* TargetFoundSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* ReloadingSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* FriendlyDownSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* EnemyDownSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* RecruitSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* AcknowledgeSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* AcknowledgeCommandSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* FollowSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* AttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* DefendSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* DeathFallSound;
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
		TArray<TSubclassOf<ALoadout>> Loadouts;
	ALoadout* loadoutObj;

	UPROPERTY(EditAnywhere, Category = "Accessories")
		TArray<TSubclassOf< AHeadgear>> Headgears;
	AHeadgear* headgearObj;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Battle Chatters", meta = (AllowPrivateAccess = "true"))
		TArray<FVoiceClipSet> VoiceClips;
	FVoiceClipSet SelectedVoiceClipSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ACombatCharacter> OperativeCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ACommanderCharacter> CommanderCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicles", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AAircraft> AC130Class;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props", meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* FlagMaterial;

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


	TSubclassOf<ACombatCharacter> GetOperativeCharacterClass() {
		return OperativeCharacterClass;
	}
	TSubclassOf<ACommanderCharacter> GetCommanderCharacterClass() {
		return CommanderCharacterClass;
	}

	TSubclassOf<AAircraft> GetAC130Class() {
		return AC130Class;
	}

	UMaterialInterface* GetFlagMaterial() {
		return FlagMaterial;

	}
};

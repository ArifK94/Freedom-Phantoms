#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "StructCollection.h"
#include "FactionManager.generated.h"

class AWeapon;
class UWeaponSet;
class USoundBase;
class AHeadgear;
class ALoadout;
class AAircraft;
class ACombatCharacter;
class ACommanderCharacter;



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

	UWeaponSet* getWeaponSetObj() { return WeaponSetObj; }

	AWeapon* getPrimaryWeaponObj() { return PrimaryWeaponObj; }
	AWeapon* getSecondaryWeaponObj() { return SecondaryWeaponObj; }


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

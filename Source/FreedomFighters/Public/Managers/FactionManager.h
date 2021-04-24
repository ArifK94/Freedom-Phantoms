#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "StructCollection.h"
#include "FactionManager.generated.h"

class AWeapon;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ACombatCharacter> OperativeCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ACommanderCharacter> CommanderCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicles", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AAircraft> AC130Class;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props", meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* FlagMaterial;

public:
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

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StructCollection.h"
#include "Engine/DataTable.h"
#include "GameModeManager.generated.h"

class ALevelManager;
class AWeapon;

UCLASS()
class FREEDOMPHANTOMS_API AGameModeManager : public AGameModeBase
{
	GENERATED_BODY()

private:
	UPROPERTY()
		TArray<FWorldCoverPoint> CoverPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		ALevelManager* LevelManager;

	/**
	* Hold a list of weapons which have been dropped from the dead characters.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<AWeapon*> DroppedWeapons;

private:
	virtual void BeginPlay() override;

	void FindLevelManager();

public:
	bool IsCoverPointTaken(FWorldCoverPoint CoverLocation);
	
	void AddCoverPoint(FWorldCoverPoint CoverLocation);

	void RemoveCoverPoint(FWorldCoverPoint CoverLocation);

	void AddDroppedWeapon(AWeapon* Weapon);


	ALevelManager* GetLevelManager() {
		return LevelManager;
	}


};

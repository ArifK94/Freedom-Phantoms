// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/DataTable.h"
#include "GameModeManager.generated.h"

class ALevelManager;

USTRUCT(BlueprintType)
struct FWorldCoverPoint 
{
	GENERATED_USTRUCT_BODY()

public:
	AActor* Owner;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FVector Location;

	FWorldCoverPoint()
	{
	}
};
UCLASS()
class FREEDOMFIGHTERS_API AGameModeManager : public AGameModeBase
{
	GENERATED_BODY()

private:
	TArray<FWorldCoverPoint> CoverPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		ALevelManager* LevelManager;

private:
	virtual void BeginPlay() override;

	void FindLevelManager();

public:
	bool IsCoverPointTaken(FWorldCoverPoint CoverLocation);
	
	void AddCoverPoint(FWorldCoverPoint CoverLocation);

	void RemoveCoverPoint(FWorldCoverPoint CoverLocation);


	ALevelManager* GetLevelManager() {
		return LevelManager;
	}


};

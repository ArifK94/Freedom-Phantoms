// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameModeManager.generated.h"

USTRUCT(BlueprintType)
struct FWorldCoverPoint : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	AActor* Owner;

	FVector Location;
};
UCLASS()
class FREEDOMFIGHTERS_API AGameModeManager : public AGameModeBase
{
	GENERATED_BODY()

private:
	TArray<FWorldCoverPoint> CoverPoints;

public:
	bool IsCoverPointTaken(FWorldCoverPoint CoverLocation);
	
	void AddCoverPoint(FWorldCoverPoint CoverLocation);

	void RemoveCoverPoint(FWorldCoverPoint CoverLocation);
};

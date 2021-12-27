// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CoverFinderComponent.generated.h"


class AGameModeManager;

class USphereComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UCoverFinderComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	AGameModeManager* GameModeManager;

	//USphereComponent* CoverSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int NumberOfCoverTraces;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CoverRadius;

public:	
	UCoverFinderComponent();

	FVector FindCover(FVector StartLocation);

	FVector GetClosestCoverPoint(TArray<FVector> CoverLocationPoints);

	bool IsCoverPointTaken(FVector PointLocation);

	virtual void BeginPlay() override;


		
};

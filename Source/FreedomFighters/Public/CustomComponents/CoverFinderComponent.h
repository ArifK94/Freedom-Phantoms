// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CoverFinderComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UCoverFinderComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	class AGameModeManager* GameModeManager;
	class AController* Controller;
	class APawn* Pawn;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int NumberOfCoverTraces;

	/**
	* The length og the line trace to find cover.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CoverLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CoverRadius;

public:	
	UCoverFinderComponent();

	bool FindCover(FVector StartLocation, FVector& ChosenCoverPoint);

	/**
	* Find cover around target actor.
	*/
	bool FindCover(AActor* TargetActor, FVector& ChosenCoverPoint);


	FVector GetClosestCoverPoint(TArray<FVector> CoverLocationPoints);

	bool IsCoverPointTaken(FVector PointLocation);

	virtual void BeginPlay() override;

private:
	void Init();

		
};

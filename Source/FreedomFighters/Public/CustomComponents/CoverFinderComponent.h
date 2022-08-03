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
	class ABaseCharacter* Character;

	FVector MyChosenCoverPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int NumberOfCoverTraces;

	/**
	* The length og the line trace to find cover.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CoverLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CoverRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CoverDistance;

public:	
	UCoverFinderComponent();

	bool FindCover(FVector StartLocation, FVector& ChosenCoverPoint);

	/**
	* Find cover around target actor.
	*/
	bool FindCover(AActor* TargetActor, FVector& ChosenCoverPoint);


	FVector GetClosestCoverPoint(TArray<FVector> CoverLocationPoints);

	bool IsCoverPointTaken(FVector PointLocation);

	void GetCorners(FVector WallNormal, FVector CoverLocation, bool& LineTraceLeft, bool& LineTraceRight);

	/**
	* Can character peak up while in cover?
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool CanCoverPeakUp(FVector WallNormal, FVector CoverLocation);

	/**
	* Is the cover a corner or can character peak from cover?
	*/
	bool IsPreferredCover(FVector WallNormal, FVector CoverLocation);

private:
	void Init();

	virtual void BeginPlay() override;

	/**
	* Get a list of potential cover points
	*/
	TArray<FVector> GetCoverPoints();

		
};

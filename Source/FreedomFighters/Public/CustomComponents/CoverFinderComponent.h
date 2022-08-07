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
	* The radius of the line trace to find cover.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CoverRadius;
	float DefaultSearchRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CoverDistance;

public:	
	UCoverFinderComponent();

	bool FindCover(FVector StartLocation, FTransform& ChosenCoverPoint);

	/**
	* Find cover around target actor.
	*/
	bool FindCover(AActor* TargetActor, FTransform& ChosenCoverPoint);


	bool IsCoverPointTaken(FVector PointLocation);

	void GetCorners(FVector WallNormal, FVector CoverLocation, bool& LineTraceLeft, bool& LineTraceRight);

	/**
	* Can character peak up while in cover?
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool CanCoverPeakUp(FVector WallNormal, FVector CoverLocation);

	/**
	* Rmoeve cover point regiestered to the owner.
	*/
	void RemoveCoverPoint();

private:
	void Init();

	virtual void BeginPlay() override;

	/**
	* Get a list of potential cover points
	*/
	TArray<FTransform> GetCoverPoints();


	/**
	* Get the closest cover point to move to.
	*/
	FTransform GetClosestCoverPoint(TArray<FTransform> CoverLocationPoints);


	/**
	* Is the cover a corner or can character peak from cover?
	*/
	bool IsPreferredCover(FVector WallNormal, FVector CoverLocation);


public:
	void SetSearchRadius(float Radius) { CoverRadius = Radius; }

	/**
	* Sets the cover radius back to default radius.
	*/
	void ResetSearchRadius();
		
};

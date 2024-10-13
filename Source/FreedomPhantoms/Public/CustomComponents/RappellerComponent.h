// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "StructCollection.h"
#include "RappellerComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRappelUpdateSignature, FRappellingParameters, RappellingInfo);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FREEDOMPHANTOMS_API URappellerComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	/** Timeline curve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rappelling", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* RappelCurve;

	UPROPERTY()
	FTimeline CurveTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rappelling", meta = (AllowPrivateAccess = "true"))
	FVector StartPosition;

	/** Start and End positions for the rappel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rappelling", meta = (AllowPrivateAccess = "true"))
	FVector EndPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rappelling", meta = (AllowPrivateAccess = "true"))
	float RappelDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rappelling", meta = (AllowPrivateAccess = "true"))
	class ARope* Rope;

public:
	URappellerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

public:
	/** Update function for the timeline */
	UFUNCTION()
	void UpdateRappelProgress(float Value);

	/** Function to handle the end of the rappel */
	UFUNCTION()
	void OnRappelFinished();

	/** Initializes the rappel setup */
	UFUNCTION(BlueprintCallable)
	void InitializeRappel(ARope* RopeActor);

	UPROPERTY(BlueprintAssignable, Category = "Rappelling")
	FOnRappelUpdateSignature OnRappelChanged;
};

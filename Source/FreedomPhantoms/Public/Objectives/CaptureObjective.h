// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objectives/BaseObjective.h"
#include "CaptureObjective.generated.h"

class AGameStateBaseCustom;
class UHealthComponent;
UCLASS()
class FREEDOMPHANTOMS_API ACaptureObjective : public ABaseObjective
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CaptureRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsPlayerCapturing;

	FTimerHandle THandler_CaptureProgress;

	AGameStateBaseCustom* GameStateBaseCustom;
	UHealthComponent* HealthComponent;

public:
	ACaptureObjective();

private:
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	void UpdateCaptureProgress();

protected:
	virtual void BeginPlay() override;
	
};

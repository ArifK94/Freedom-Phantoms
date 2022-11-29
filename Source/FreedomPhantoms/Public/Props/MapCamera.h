// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StructCollection.h"
#include "MapCamera.generated.h"

class USceneCaptureComponent2D;
class ATargetSystemMarker;
UCLASS()
class FREEDOMPHANTOMS_API AMapCamera : public AActor
{
	GENERATED_BODY()
	
private:
	FVector LocationInput;
	bool StartPostTimer;
	float CurrentPostActivateTimer;

	/** When camera hits a collisiom, the zoom needs to be corrected to move up/ */
	float ZoomCorrectionZ;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USceneCaptureComponent2D* SceneCaptureComponent2D;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ATargetSystemMarker> FriendlyMarkerClass;
	TArray<FTargetSystemNode*> FriendlyMarkerNodes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ATargetSystemMarker> EnemyMarkerClass;
	TArray<FTargetSystemNode*> EnemyMarkerNodes;

	/** Go back to default location after panning around or zooming in */
	FVector DefaultLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float PanningSpeedMultiplier;

	/** Min X and Y Panning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVector2D PanningMin;

	/** Max X and Y Panning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVector2D PanningMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float ZoomSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float ZoomMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float ZoomMax;

	/** Locking input for pause menu, when switching to different tabs, the movement should not take place  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool LockInput;

public:	
	AMapCamera();

	void Activate();

	void Deactivate();

	void PostActivate();

	UFUNCTION(BlueprintCallable)
		void MoveToPlayerLocation();

	UFUNCTION(BlueprintCallable)
		void MoveForward(float Value);

	UFUNCTION(BlueprintCallable)
		void MoveRight(float Value);

	UFUNCTION(BlueprintCallable)
		void Zoom(float Value);

private:
	void CheckCollision();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DelatTime) override;


public:
	FVector2D GetPanningMin() { return PanningMin; }

	FVector2D GetPanningMax() { return PanningMax; }

	float GetZoomMin() { return ZoomMin; }

	float GetZoomMax() { return ZoomMax; }

};

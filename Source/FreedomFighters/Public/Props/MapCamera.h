// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StructCollection.h"
#include "MapCamera.generated.h"

class ATargetSystemMarker;
UCLASS()
class FREEDOMFIGHTERS_API AMapCamera : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UPostProcessComponent* PostProcessor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ATargetSystemMarker> FriendlyMarkerClass;
	TArray<FTargetSystemNode*> FriendlyMarkerNodes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ATargetSystemMarker> EnemyMarkerClass;
	TArray<FTargetSystemNode*> EnemyMarkerNodes;

public:	
	AMapCamera();

protected:
	virtual void BeginPlay() override;

};

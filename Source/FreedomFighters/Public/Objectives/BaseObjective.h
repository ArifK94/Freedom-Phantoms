// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseObjective.generated.h"

class AGameStateBaseCustom;
class UBoxComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompletedSignature, ABaseObjective*, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdateSignature, ABaseObjective*, Objective, float, Progress);
UCLASS()
class FREEDOMFIGHTERS_API ABaseObjective : public AActor
{
	GENERATED_BODY()

public:
	FOnObjectiveCompletedSignature OnObjectiveCompleted;
	FOnObjectiveUpdateSignature OnObjectiveUpdate;

protected:
	AGameStateBaseCustom* GameStateBaseCustom;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* BoxCollider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		FName Objective;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		bool IsObjectiveComplete;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		bool IsFinalObjective;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float Progress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* ObjectiveCompleteSound;	
public:	
	ABaseObjective();

private:
	void AddToPlayer();

protected:
	virtual void BeginPlay() override;

	void ObjectiveComplete();

public:
	bool GetIsFinalObjective() {
		return IsFinalObjective;
	}

	float GetProgress() {
		return Progress;
	}

};

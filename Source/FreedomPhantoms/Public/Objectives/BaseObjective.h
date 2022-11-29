// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseObjective.generated.h"

class AGameStateBaseCustom;
class UBoxComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompletedSignature, ABaseObjective*, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveInteractedSignature, ABaseObjective*, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdateSignature, ABaseObjective*, Objective, float, Progress);
UCLASS()
class FREEDOMPHANTOMS_API ABaseObjective : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnObjectiveCompletedSignature OnObjectiveCompleted;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnObjectiveInteractedSignature OnObjectiveInteracted;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnObjectiveUpdateSignature OnObjectiveUpdate;

protected:
	AGameStateBaseCustom* GameStateBaseCustom;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* BoxCollider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName ObjectiveMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsObjectiveComplete;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsFinalObjective;

	/** Is the objective optional? If so, then if complete, this objective will not affect the overall compulsory objectives. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsOptional;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float Progress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* ObjectiveCompleteSound;	
public:	
	ABaseObjective();

protected:
	void ObjectiveComplete();

public:
	bool GetIsFinalObjective() {
		return IsFinalObjective;
	}

	float GetProgress() {
		return Progress;
	}

	bool GetIsOptional() { return IsOptional; }

};

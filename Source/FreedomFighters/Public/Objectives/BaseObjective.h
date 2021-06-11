// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseObjective.generated.h"

class UBoxComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompletedSignature, ABaseObjective*, Objective);
UCLASS()
class FREEDOMFIGHTERS_API ABaseObjective : public AActor
{
	GENERATED_BODY()

public:
	FOnObjectiveCompletedSignature OnObjectiveCompleted;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* BoxCollider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		FName Objective;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		bool IsObjectiveComplete;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* ObjectiveCompleteSound;	
public:	
	ABaseObjective();

protected:
	virtual void BeginPlay() override;

	void ObjectiveComplete();
};

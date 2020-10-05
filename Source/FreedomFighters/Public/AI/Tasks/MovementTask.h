// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "MovementTask.generated.h"

class ABaseCharacter;
class ACommanderCharacter;
UCLASS(Blueprintable)
class FREEDOMFIGHTERS_API UMovementTask : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

		void ReceiveTickAI(AAIController* OwnerController, APawn* ControlledPawn, float DeltaSeconds);

	UMovementTask(const FObjectInitializer& ObjectInit);


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FBlackboardKeySelector BB_TargetDestination;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float AcceptanceRadius;

private:
	AAIController* OwningController;

	ACommanderCharacter* Commander;

	ABaseCharacter* OwningCharacter;

	FVector TargetLocation;



};

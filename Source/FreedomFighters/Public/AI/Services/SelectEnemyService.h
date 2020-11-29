// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "SelectEnemyService.generated.h"


UCLASS(Abstract, Blueprintable)
class FREEDOMFIGHTERS_API USelectEnemyService : public UBTService
{
	GENERATED_BODY()

		void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FBlackboardKeySelector BB_TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TargetSightDistance = 4000.0f;

};

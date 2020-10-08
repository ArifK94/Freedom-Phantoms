// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "CheckOrderService.generated.h"

UCLASS(Abstract, Blueprintable)
class FREEDOMFIGHTERS_API UCheckOrderService : public UBTService
{
	GENERATED_BODY()
	
		void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FBlackboardKeySelector BB_OrderType;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "Characters/CommanderCharacter.h"

#include "OrderTaskNode.generated.h"

UCLASS(Abstract, Blueprintable)
class FREEDOMFIGHTERS_API UOrderTaskNode : public UBTTaskNode
{
	GENERATED_BODY()


	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	UOrderTaskNode();


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FBlackboardKeySelector BB_TargetDestination;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		CommanderOrders CurrentCommand;

private:
	UBlackboardComponent* BlackboardComp;

private:
	void DefendOrder(FVector DefendLocation);

	void FollowOrder(FVector DefendLocation);

	void AttackOrder(FVector DefendLocation);

};

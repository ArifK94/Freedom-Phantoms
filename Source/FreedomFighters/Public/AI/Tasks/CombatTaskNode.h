// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CombatTaskNode.generated.h"

class ACombatCharacter;


UCLASS(Abstract, Blueprintable)
class FREEDOMFIGHTERS_API UCombatTaskNode : public UBTTaskNode
{
	GENERATED_BODY()
		
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FBlackboardKeySelector BB_EnemyActor;

	ACombatCharacter* OwningCombatCharacter;

	FTimerHandle THandler_TimeBetweenShots;

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void EndFiring();

};

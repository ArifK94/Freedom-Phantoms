// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CombatTaskNode.generated.h"

class ACombatCharacter;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class CombatState : uint8
{
	Aiming		UMETA(DisplayName = "Aiming"),
	Shooting 		UMETA(DisplayName = "Shooting"),
};


UCLASS(Abstract, Blueprintable)
class FREEDOMFIGHTERS_API UCombatTaskNode : public UBTTaskNode
{
	GENERATED_BODY()

		virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		CombatState CombatMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsInCombat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FBlackboardKeySelector BB_EnemyActor;

private:

	FTimerHandle THandler_TimeBetweenShots;

	ACombatCharacter* OwningCharacter;
private:
	void EndFiring();
	
};

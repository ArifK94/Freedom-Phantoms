// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Runtime/Engine/Classes/Engine/Blueprint.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "UtilityAIAction.generated.h"

/**
 *
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class FREEDOMPHANTOMS_API UUtilityAIAction : public UObject
{
	GENERATED_BODY()

private:
	bool bMarkedForDeath;

	FTimerHandle THandler_TimeOut;
	bool bIsTimedOut;

protected:
	UPROPERTY()
		class ACombatAIController* CombatAIController;

	UPROPERTY()
		class ACombatCharacter* OwningCombatCharacter;

public:

	UPROPERTY(BlueprintReadOnly)
		float LastScore;

	UPROPERTY(BlueprintReadOnly)
		bool LastCanRun;

	UPROPERTY(BlueprintReadWrite)
		float CooldownAmount;


public:
	UUtilityAIAction();

	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Spawn"))
		void ReceiveSpawn(AAIController* Controller, APawn* Pawn);

	virtual void Spawn(AAIController* Controller, APawn* Pawn);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Tick"))
		void ReceiveTick(float DeltaTime, AAIController* Controller, APawn* Pawn);

	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn);

	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "Can Run"))
		bool ReceiveCanRun(AAIController* Controller, APawn* Pawn) const;

	bool ReceiveCanRun_Implementation(AAIController* Controller, APawn* Pawn) const { return true; }

	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const;


	/**
	* Should this action be run at the same time with other actions? if so, then the score is not considered for async actions.
	*/
	virtual bool CanRunAsynchronously(AAIController* Controller, APawn* Pawn) const;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Score"))
		float ReceiveScore(AAIController* Controller, APawn* Pawn);

	virtual float Score(AAIController* Controller, APawn* Pawn);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Enter"))
		void ReceiveEnter(AAIController* Controller, APawn* Pawn);

	virtual void Enter(AAIController* Controller, APawn* Pawn);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Exit"))
		void ReceiveExit(AAIController* Controller, APawn* Pawn);

	virtual void Exit(AAIController* Controller, APawn* Pawn);


	UFUNCTION(BlueprintCallable)
		void Kill();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool IsMarkedForDeath();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool IsTimedOut();

	UFUNCTION(BlueprintCallable)
		void Resurrect();

	UFUNCTION(BlueprintCallable)
		void StartTimeOut();

	UFUNCTION(BlueprintCallable)
		void StopTimeOut();
};


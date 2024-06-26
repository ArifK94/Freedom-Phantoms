// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomComponents/Engine/MyActorComponent.h"
#include "Runtime/Core/Public/Math/RandomStream.h"
#include "AI/UtilityAIAction.h"
#include "UtilityAIComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUtilityAIActionSpawned, UUtilityAIAction*, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUtilityAIActionChanged, UUtilityAIAction*, NewAction, UUtilityAIAction*, OldAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUtilityAIActionNotAvailable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUtilityAIInitialized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUtilityAIBeforeScoreComputation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUtilityAIActionChoosen, UUtilityAIAction*, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUtilityAIActionTicked, UUtilityAIAction*, Action);

UCLASS(BlueprintType, Blueprintable, ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class FREEDOMPHANTOMS_API UUtilityAIComponent : public UMyActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UUtilityAIComponent();

protected:
	virtual void BeginPlay() override;

	virtual void TimerTick() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI", meta = (ExposeOnSpawn = "true"))
		TSet<TSubclassOf<UUtilityAIAction>> Actions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
		bool bIgnoreZeroScore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
		bool bUseLowestScore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
		bool bInvertPriority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
		bool bRandomizeOnEquality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
		float EqualityTolerance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
		float Bounciness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
		bool bCanRunWithoutPawn;


	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On UtilityAI Action Spawned"))
		FUtilityAIActionSpawned OnUtilityAIActionSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On UtilityAI Action Changed"))
		FUtilityAIActionChanged OnUtilityAIActionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On UtilityAI Action Not Available"))
		FUtilityAIActionNotAvailable OnUtilityAIActionNotAvailable;

	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On UtilityAI Initialized"))
		FUtilityAIInitialized OnUtilityAIInitialized;

	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On UtilityAI Action Choosen"))
		FUtilityAIActionChoosen OnUtilityAIActionChoosen;

	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On UtilityAI Action Ticked"))
		FUtilityAIActionChoosen OnUtilityAIActionTicked;

	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On UtilityAI Before Score Computation"))
		FUtilityAIBeforeScoreComputation OnUtilityAIBeforeScoreComputation;

	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "Compute Best Action"))
		UUtilityAIAction* ReceiveComputeBestAction(AAIController* Controller, APawn* Pawn);

	UUtilityAIAction* ReceiveComputeBestAction_Implementation(AAIController* Controller, APawn* Pawn);

	virtual UUtilityAIAction* ComputeBestAction(AAIController* Controller, APawn* Pawn);


	UPROPERTY()
		TSet<UUtilityAIAction*> InstancedActions;

	/**
	* Actions which can run asynchronously with another action.
	*/
	UPROPERTY()
		TSet<UUtilityAIAction*> InstancedAsyncActions;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
		TArray<UUtilityAIAction*> GetActionInstances() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
		UUtilityAIAction* GetActionInstanceByClass(TSubclassOf<UUtilityAIAction> ActionClass) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
		UUtilityAIAction* GetCurrentActionInstance() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
		TSubclassOf<UUtilityAIAction> GetCurrentActionClass() const;

	UFUNCTION(BlueprintCallable, Category = "Utility AI")
		UUtilityAIAction* SpawnActionInstance(TSubclassOf<UUtilityAIAction> ActionClass);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
		bool CanSpawnActionInstance(TSubclassOf<UUtilityAIAction> ActionClass) const;

	UFUNCTION(BlueprintCallable, Category = "Utility AI")
		void SetRandomStream(FRandomStream InRandomStream);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
		FRandomStream GetRandomStream() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Utility AI", meta = (DisplayName = "Score Filter"))
		float ScoreFilter(UUtilityAIAction* Action, float Score) const;

	float ScoreFilter_Implementation(UUtilityAIAction* Action, float Score) const { return Score; }

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI", meta = (AllowPrivateAccess = "true"))
		bool EnableUtilityAI;

protected:

	UUtilityAIAction* LastAction;
	APawn* LastPawn;
	float LastSwitchTime;

	bool bUseRandomStream;
	FRandomStream RandomStream;

	bool InternalRandBool() const;

	bool CheckLowestScore(UUtilityAIAction* Current, UUtilityAIAction* Best) const;
	bool CheckHighestScore(UUtilityAIAction* Current, UUtilityAIAction* Best) const;

public:
	void SetEnableUtilityAI(bool Enabled);
};

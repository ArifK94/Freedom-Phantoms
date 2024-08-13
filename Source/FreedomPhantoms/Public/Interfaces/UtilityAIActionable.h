// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UtilityAIActionable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUtilityAIActionable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FREEDOMPHANTOMS_API IUtilityAIActionable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool CanInteract(APawn* InPawn, AController* InController);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		float Score(AAIController* Controller, APawn* Pawn);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool CanRun(AAIController* Controller, APawn* Pawn) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void Spawn(AAIController* Controller, APawn* Pawn);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void Enter(AAIController* Controller, APawn* Pawn);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void Exit(AAIController* Controller, APawn* Pawn);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn);
};

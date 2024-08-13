// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FREEDOMPHANTOMS_API IInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent) // For blueprint implementation
		FString GetKeyDisplayName();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		FString OnInteractionFound(APawn* InPawn, AController* InController);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		AActor* OnPickup(APawn* InPawn, AController* InController);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool OnUseInteraction(APawn* InPawn, AController* InController);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		bool CanInteract(APawn* InPawn, AController* InController);

};

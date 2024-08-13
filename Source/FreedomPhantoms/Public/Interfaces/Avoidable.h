// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StructCollection.h"
#include "Avoidable.generated.h"

UINTERFACE(MinimalAPI)
class UAvoidable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Actors which should be avoided eg. Grenades
 */
class FREEDOMPHANTOMS_API IAvoidable
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void OnNearbyActorFound(FAvoidableParams AvoidableParams);
};

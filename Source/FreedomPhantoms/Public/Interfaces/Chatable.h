// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StructCollection.h"
#include "Chatable.generated.h"

UINTERFACE(MinimalAPI)
class UChatable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FREEDOMPHANTOMS_API IChatable
{
	GENERATED_BODY()

public:

	/**
	* When an object has communicated and waiting for a repsonse.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void OnCallerReceived(FChatableParams ChatableParams);
};

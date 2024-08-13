// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controllers/CustomPlayerController.h"
#include "UnarmedPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMPHANTOMS_API AUnarmedPlayerController : public ACustomPlayerController
{
	GENERATED_BODY()
	
private:
	virtual void InitInputComponent() override;

	virtual void InitBeginPlayUncommon() override;


	virtual void OnPossess(APawn* InPawn) override;

};

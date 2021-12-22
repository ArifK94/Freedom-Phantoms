// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetPointComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UTargetPointComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTargetPointComponent();

protected:
	virtual void BeginPlay() override;

public:	

		
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SupportPackage.generated.h"

UCLASS()
class FREEDOMFIGHTERS_API ASupportPackage : public AActor
{
	GENERATED_BODY()
	
public:	
	ASupportPackage();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};

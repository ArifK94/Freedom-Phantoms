// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightVisionGoggle.generated.h"

UCLASS()
class FREEDOMFIGHTERS_API ANightVisionGoggle : public AActor
{
	GENERATED_BODY()
	
public:	
	ANightVisionGoggle();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Night Vision Goggles", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};

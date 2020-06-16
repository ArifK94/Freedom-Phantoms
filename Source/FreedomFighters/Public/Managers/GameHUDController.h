// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "Components/WidgetComponent.h"


#include "GameHUDController.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMFIGHTERS_API AGameHUDController : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Interactive")
		TSubclassOf<UUserWidget> HitComboWidgetClass;

private:
	UUserWidget* HitComboWidget;


public:
	AGameHUDController();

	void CreateNVGWidget();
	
};

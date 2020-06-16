// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/GameHUDController.h"

AGameHUDController::AGameHUDController()
{

}


void AGameHUDController::CreateNVGWidget()
{
	if (HitComboWidgetClass)
	{
		HitComboWidget = CreateWidget<UUserWidget>(GetWorld(), HitComboWidgetClass);
		/** Make sure widget was created */
		if (HitComboWidget)
		{
			/** Add it to the viewport */
			HitComboWidget->AddToViewport();
		}
	}
}
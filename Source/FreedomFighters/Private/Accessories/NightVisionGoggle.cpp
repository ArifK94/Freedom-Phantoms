// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessories/NightVisionGoggle.h"

#include "Components/StaticMeshComponent.h"
#include "Components/PostProcessComponent.h"

ANightVisionGoggle::ANightVisionGoggle()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	Mesh->CanCharacterStepUpOn = ECB_No;

	VisionPPComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("VisionPPComp"));

	isVisorOn = false;

}


void ANightVisionGoggle::ToggleVision()
{
	isVisorOn = !isVisorOn;
}


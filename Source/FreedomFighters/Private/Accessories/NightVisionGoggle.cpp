// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessories/NightVisionGoggle.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/PostProcessComponent.h"

#include "Blueprint/UserWidget.h"


ANightVisionGoggle::ANightVisionGoggle()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	Mesh->CanCharacterStepUpOn = ECB_No;

	VisionPPComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("VisionPPComp"));

	isVisorOn = false;
}

void ANightVisionGoggle::BeginPlay()
{
	Super::BeginPlay();

	VisionPPComp->bEnabled = false;
}

void ANightVisionGoggle::ToggleVision()
{
	isVisorOn = !isVisorOn;

	if (isVisorOn)
	{
		VisionPPComp->bEnabled = true;
	}
	else
	{
		VisionPPComp->bEnabled = false;
	}
}


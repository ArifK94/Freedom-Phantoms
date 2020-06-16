// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessories/NightVisionGoggle.h"
#include "Managers/GameHUDController.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/AudioComponent.h"


ANightVisionGoggle::ANightVisionGoggle()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	Mesh->CanCharacterStepUpOn = ECB_No;

	VisionPPComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("VisionPPComp"));

	NVGAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NVGAudioComponent"));

	isVisorOn = false;
}

void ANightVisionGoggle::BeginPlay()
{
	Super::BeginPlay();

	gameHUDController = Cast<AGameHUDController>(GetWorld()->GetFirstPlayerController()->GetHUD());
	VisionPPComp->bEnabled = false;
}


void ANightVisionGoggle::ToggleVision()
{
	isVisorOn = !isVisorOn;

	if (gameHUDController)
	{
		gameHUDController->CreateNVGWidget();
	}

	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ANightVisionGoggle::SetVisorState, .5f, false);
}



void ANightVisionGoggle::SetVisorState()
{
	if (isVisorOn)
	{
		VisionPPComp->bEnabled = true;

		if (NVGOnSound != NULL)
		{
			NVGAudioComponent->Sound = NVGOnSound;
			NVGAudioComponent->Play(0.0f);
		}
	}
	else
	{
		VisionPPComp->bEnabled = false;

		if (NVGOffSound != NULL)
		{
			NVGAudioComponent->Sound = NVGOffSound;
			NVGAudioComponent->Play(0.0f);
		}
	}
}
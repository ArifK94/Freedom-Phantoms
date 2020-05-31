// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessories/Headgear.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Math/UnrealMathUtility.h"

// Sets default values
AHeadgear::AHeadgear()
{
	CreateStaticMeshParent();

	Nightvision_Holder = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Nightvision_Holder"));
	Nightvision_Holder->AttachTo(StaticMesh, "Nightvision_Holder");
	Nightvision_Holder->SetCollisionProfileName(TEXT("NoCollision"));
	Nightvision_Holder->CanCharacterStepUpOn = ECB_No;

	Nightvision_Goggles = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Nightvision_Goggles"));
	Nightvision_Goggles->AttachTo(StaticMesh, "Nightvision");
	Nightvision_Goggles->SetCollisionProfileName(TEXT("NoCollision"));
	Nightvision_Goggles->CanCharacterStepUpOn = ECB_No;

	Goggles = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Goggles"));
	Goggles->AttachTo(StaticMesh, "Goggles");
	Goggles->SetCollisionProfileName(TEXT("NoCollision"));
	Goggles->CanCharacterStepUpOn = ECB_No;

	Torchlight_Holder = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Torchlight_Holder"));
	Torchlight_Holder->AttachTo(StaticMesh, "Torchlight_Holder");
	Torchlight_Holder->SetCollisionProfileName(TEXT("NoCollision"));
	Torchlight_Holder->CanCharacterStepUpOn = ECB_No;

	Torchlight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Torchlight"));
	Torchlight->AttachTo(StaticMesh, "Torchlight");
	Torchlight->SetCollisionProfileName(TEXT("NoCollision"));
	Torchlight->CanCharacterStepUpOn = ECB_No;

	TorchBeam = CreateDefaultSubobject<USpotLightComponent>(TEXT("TorchBeam"));
	TorchBeam->AttachTo(StaticMesh, "TorchBeam");

	LaserBeam = CreateDefaultSubobject<USpotLightComponent>(TEXT("LaserBeam"));
	LaserBeam->AttachTo(StaticMesh, "LaserBeam");

	isNightVisionOn = false;
	isGogglesOff = false;
	ParentSocket = "headgear_socket";

}

// Toggle visibility between goggles & night vision goggles
void AHeadgear::ToggleRandomAccessory()
{
	int random = FMath::RandRange(0, 1);

	switch (random)
	{
	case 0:
		Nightvision_Holder->SetVisibility(false);
		Nightvision_Goggles->SetVisibility(false);
		break;
	case 1:
		Goggles->SetVisibility(false);
		break;
	default:
		Nightvision_Holder->SetVisibility(false);
		Nightvision_Goggles->SetVisibility(false);
		break;
	}
}

void AHeadgear::ToggleVisor()
{
	FRotator VisorAngle;

	if (isNightVisionOn)
	{
		VisorAngle.Roll = 0.0f;
		isNightVisionOn = false;
	}
	else
	{
		VisorAngle.Roll = 140.0f;
		isNightVisionOn = true;
	}

	Nightvision_Goggles->SetRelativeRotation(VisorAngle);

}

void AHeadgear::ToggleGoggles()
{
	FRotator GogglesAngle;

	if (isGogglesOff)
	{
		GogglesAngle.Roll = 0.0f;
		isGogglesOff = false;
	}
	else
	{
		GogglesAngle.Roll = -30.0f;
		isGogglesOff = true;
	}

	Goggles->SetRelativeRotation(GogglesAngle);

}
// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessories/Headgear.h"
#include "Accessories/NightVisionGoggle.h"
#include "Accessories/Goggle.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Math/UnrealMathUtility.h"

AHeadgear::AHeadgear()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	Mesh->CanCharacterStepUpOn = ECB_No;

	ParentSocket = "headgear_socket";
	GoggleSocket = "Goggles";
	NVGSocket = "NVG";


}

void AHeadgear::BeginPlay()
{
	Super::BeginPlay();

	SpawnGoggle();
	SpawnNVG();
}

void AHeadgear::SpawnNVG()
{
	UWorld* world = GetWorld();

	if (world)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Spawn the weapon actor
		NightVisionGoggleObj = world->SpawnActor<ANightVisionGoggle>(NightVisionGoggle, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (NightVisionGoggleObj)
		{
			NightVisionGoggleObj->SetOwner(this);
			NightVisionGoggleObj->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NVGSocket);
		}
	}
}



void AHeadgear::SpawnGoggle()
{
	UWorld* world = GetWorld();

	if (world)
	{

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Spawn the weapon actor
		GoggleObj = world->SpawnActor<AGoggle>(Goggle, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (GoggleObj)
		{
			GoggleObj->SetOwner(this);
			GoggleObj->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, GoggleSocket);
		}
	}
}




// Toggle visibility between goggles & night vision goggles
void AHeadgear::ToggleRandomAccessory()
{
	int random = FMath::RandRange(0, 1);

	switch (random)
	{
	case 0:
	//	Nightvision_Goggles->SetVisibility(false);
		break;
	case 1:
	//	Goggles->SetVisibility(false);
		break;
	default:
	//	Nightvision_Goggles->SetVisibility(false);
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
}
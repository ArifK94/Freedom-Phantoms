// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessories/Headgear.h"
#include "Accessories/NightVisionGoggle.h"
#include "Accessories/Goggle.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "Components/SpotLightComponent.h"
#include "Math/UnrealMathUtility.h"

AHeadgear::AHeadgear()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
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
	ToggleRandomAccessory();
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
	int RandomAccessory = FMath::RandRange(0, 1);

	switch (RandomAccessory)
	{
	case 1: // show nightvision goggles
		NightVisionGoggleObj->GetMesh()->SetVisibility(true);
		GoggleObj->GetMesh()->SetVisibility(true);
		break;
	default: // show goggles
		NightVisionGoggleObj->GetMesh()->SetVisibility(false);
		GoggleObj->GetMesh()->SetVisibility(true);

		int RandomGoggleVisor = FMath::RandRange(0, 1);

		if (RandomAccessory == 0)
		{
			IsGoggleOff = true;
		}
		else
		{
			IsGoggleOff = false;
		}
		break;
	}
}

void AHeadgear::ToggleGoggles()
{
	IsGoggleOff = !IsGoggleOff;
}
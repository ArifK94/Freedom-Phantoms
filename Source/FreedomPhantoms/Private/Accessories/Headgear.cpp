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
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	ParentSocket = "headgear_socket";
	GoggleSocket = "Goggles";
	NVGSocket = "NVG";

	IsGoggleOn = false;
}

void AHeadgear::BeginPlay()
{
	Super::BeginPlay();

	IsGoggleOn = true;

	SpawnGoggle();
	SpawnNVG();
	ToggleRandomAccessory();
}

void AHeadgear::SpawnNVG()
{
	if (NightVisionGoggleClass == nullptr) {
		return;
	}

	UWorld* World = GetWorld();

	if (World == nullptr) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn the weapon actor
	NightVisionGoggleObj = World->SpawnActor<ANightVisionGoggle>(NightVisionGoggleClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (NightVisionGoggleObj)
	{
		NightVisionGoggleObj->SetOwner(this);
		NightVisionGoggleObj->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NVGSocket);
	}

}


void AHeadgear::SpawnGoggle()
{
	if (GoggleClass == nullptr) {
		return;
	}

	UWorld* World = GetWorld();

	if (World == nullptr) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn the weapon actor
	GoggleObj = World->SpawnActor<AGoggle>(GoggleClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (GoggleObj)
	{
		GoggleObj->SetOwner(this);
		GoggleObj->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, GoggleSocket);
	}

}


// Toggle visibility between goggles & night vision goggles
void AHeadgear::ToggleRandomAccessory()
{
	int RandomAccessory = FMath::RandRange(0, 1);

	if (RandomAccessory == 0)
	{
		// show nightvision goggles
		if (NightVisionGoggleObj) {
			EnableActor(NightVisionGoggleObj, true);
		}

		if (GoggleObj) {
			EnableActor(GoggleObj, true);

		}
	}
	else
	{
		// show only goggles
		if (NightVisionGoggleObj) {
			EnableActor(NightVisionGoggleObj, false);
		}

		if (GoggleObj) {
			EnableActor(GoggleObj, true);
		}

		int RandomGoggleVisor = FMath::RandRange(0, 1);

		if (RandomGoggleVisor == 0)
		{
			IsGoggleOn = true;
		}
		else
		{
			IsGoggleOn = false;
		}
	}
}

void AHeadgear::ToggleGoggles()
{
	IsGoggleOn = !IsGoggleOn;
}

void AHeadgear::EnableActor(AActor* Actor, bool IsEnabled)
{
	Actor->SetActorHiddenInGame(!IsEnabled);
	Actor->SetHidden(!IsEnabled);
	Actor->SetActorEnableCollision(IsEnabled);
	Actor->SetActorTickEnabled(IsEnabled);
}
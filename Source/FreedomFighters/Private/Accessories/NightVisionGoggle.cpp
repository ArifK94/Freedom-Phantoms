// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessories/NightVisionGoggle.h"

#include "Components/StaticMeshComponent.h"

ANightVisionGoggle::ANightVisionGoggle()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	Mesh->CanCharacterStepUpOn = ECB_No;

}

void ANightVisionGoggle::BeginPlay()
{
	Super::BeginPlay();
	
}

void ANightVisionGoggle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponLaser.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"


AWeaponLaser::AWeaponLaser()
{
	//PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComp->CanCharacterStepUpOn = ECB_No;
	MeshComp->AttachTo(RootComponent);

	LightComp = CreateDefaultSubobject<USpotLightComponent>(TEXT("LightComp"));
	LightComp->Intensity = 30000.0f;
	LightComp->AttenuationRadius = 5000.0f;
	LightComp->InnerConeAngle = 0.1f;
	LightComp->OuterConeAngle = 0.1f;
	LightComp->bUseInverseSquaredFalloff = false;

	LightSocket = "Beam";
	isLightEnabled = false;
}

void AWeaponLaser::ToggleBeam()
{
	isLightEnabled = !isLightEnabled;

	LightComp->bVisible = isLightEnabled;
}

void AWeaponLaser::BeginPlay()
{
	Super::BeginPlay();
	
	//LightComp->bVisible = isLightEnabled;
	LightComp->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LightSocket);

}

void AWeaponLaser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


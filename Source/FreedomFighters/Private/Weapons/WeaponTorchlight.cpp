// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponTorchlight.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"

AWeaponTorchlight::AWeaponTorchlight()
{
	//PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComp->CanCharacterStepUpOn = ECB_No;

	LightComp = CreateDefaultSubobject<USpotLightComponent>(TEXT("LightComp"));
	LightComp->Intensity = 30000.0f;
	LightComp->AttenuationRadius = 5000.0f;
	LightComp->InnerConeAngle = 0.1f;
	LightComp->OuterConeAngle = 0.1f;
	LightComp->bUseInverseSquaredFalloff = false;

	LightSocket = "Beam";
	isLightEnabled = false;

}

void AWeaponTorchlight::ToggleBeam()
{
	isLightEnabled = !isLightEnabled;

	LightComp->SetVisibility(isLightEnabled);
}

void AWeaponTorchlight::BeginPlay()
{
	Super::BeginPlay();
	
	LightComp->SetVisibility(isLightEnabled);
	LightComp->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LightSocket);

}

void AWeaponTorchlight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}


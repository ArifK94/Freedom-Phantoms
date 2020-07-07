// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponTorchlight.h"

#include "Components/StaticMeshComponent.h"

AWeaponTorchlight::AWeaponTorchlight()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComp->CanCharacterStepUpOn = ECB_No;
}

void AWeaponTorchlight::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeaponTorchlight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


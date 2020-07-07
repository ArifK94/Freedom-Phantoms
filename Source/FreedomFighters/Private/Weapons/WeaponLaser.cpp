// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponLaser.h"

#include "Components/StaticMeshComponent.h"

AWeaponLaser::AWeaponLaser()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComp->CanCharacterStepUpOn = ECB_No;
}

void AWeaponLaser::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeaponLaser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


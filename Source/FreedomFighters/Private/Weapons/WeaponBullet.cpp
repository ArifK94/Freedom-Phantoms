// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBullet.h"

#include "Components/StaticMeshComponent.h"

#include "GameFramework/ProjectileMovementComponent.h"

AWeaponBullet::AWeaponBullet()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	Mesh->CanCharacterStepUpOn = ECB_No;

	BulletMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("BulletMovement"));
	BulletMovement->InitialSpeed = 2000.0f;
	BulletMovement->MaxSpeed = 2000.0f;
}

void AWeaponBullet::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeaponBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


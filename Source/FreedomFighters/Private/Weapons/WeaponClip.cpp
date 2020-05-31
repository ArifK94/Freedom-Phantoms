// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponClip.h"

// Sets default values
AWeaponClip::AWeaponClip()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	clipMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClipMeshComp"));
	clipMeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	clipMeshComp->CanCharacterStepUpOn = ECB_No;

	ammoCapacity = 30;

}

// Called when the game starts or when spawned
void AWeaponClip::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponClip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


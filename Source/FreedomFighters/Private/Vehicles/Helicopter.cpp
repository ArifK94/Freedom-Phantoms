// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/Helicopter.h"

#include "Characters/BaseCharacter.h"
#include "Camera/CameraComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"

#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"

AHelicopter::AHelicopter()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	HelicopterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	HelicopterMesh->SetCollisionProfileName(TEXT("Vehicle"));
	HelicopterMesh->AttachTo(Root);

	RotorAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("RotorAudio"));
	RotorAudio->AttachTo(HelicopterMesh);
}

void AHelicopter::BeginPlay()
{
	Super::BeginPlay();

	SpawnPassenger();
}

void AHelicopter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHelicopter::SpawnPassenger()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int i = 0; i < HelicopterSeating.Num(); i++) 
	{
		FHelicopterSeating HeliSeat = HelicopterSeating[i];

		HeliSeat.CharacterObj = GetWorld()->SpawnActor<ABaseCharacter>(HeliSeat.Character, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (HeliSeat.CharacterObj)
		{
			HeliSeat.CharacterObj->AttachToComponent(HelicopterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeliSeat.SeatingSocketName);
			HeliSeat.CharacterObj->SetIsInHelicopter(true);
			HeliSeat.CharacterObj->SetHelicopterSeatPosition(HeliSeat.SeatPosition);

			FRotator CamRotation = HeliSeat.CharacterObj->FollowCamera->GetComponentRotation();

			float TargetYaw = FMath::Clamp(CamRotation.Yaw, HeliSeat.CameraViewYawMin, HeliSeat.CameraViewYawMax);

			FRotator TargetRotation = UKismetMathLibrary::MakeRotator(CamRotation.Roll, CamRotation.Pitch, TargetYaw);
		//	HeliSeat.CharacterObj->FollowCamera->SetWorldRotation(TargetRotation);
		}

	}
}

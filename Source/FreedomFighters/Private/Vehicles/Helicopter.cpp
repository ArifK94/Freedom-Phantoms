// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/Helicopter.h"

#include "Characters/BaseCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"

#include "Components/AudioComponent.h"

#include "Kismet/KismetStringLibrary.h"

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

	PassengerSeatPrefix = "DoorSitPos";
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
	if (PassengerCharacter)
	{
		const TArray<USkeletalMeshSocket*> AllSockets = HelicopterMesh->SkeletalMesh->GetActiveSocketList();

		for (int32 SocketIdx = 0; SocketIdx < AllSockets.Num(); ++SocketIdx)
		{
			FName socketName = AllSockets[SocketIdx]->SocketName;
			FString socketNameString = UKismetStringLibrary::Conv_NameToString(socketName);

			if (socketNameString.Contains(UKismetStringLibrary::Conv_NameToString(PassengerSeatPrefix)))
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


				FTransform SeatTransform = HelicopterMesh->GetSocketTransform(socketName, ERelativeTransformSpace::RTS_World);

				PassengerCharacterObj = GetWorld()->SpawnActor<ABaseCharacter>(PassengerCharacter, SeatTransform.GetLocation(), SeatTransform.GetRotation().Rotator(), SpawnParams);

				if (PassengerCharacterObj)
				{
					PassengerCharacterObj->AttachToComponent(HelicopterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, socketName);
					PassengerCharacterObj->SetIsInHelicopter(true);

					// get the position number which is the suffix
					FString position = UKismetStringLibrary::GetSubstring(socketNameString, socketNameString.Len() - 1, 1);
					PassengerCharacterObj->SetHelicopterSeatPosition(UKismetStringLibrary::Conv_StringToInt(position));
				}
			}
		}
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/SupportPackage.h"
#include "Vehicles/Aircraft.h"
#include "Characters/BaseCharacter.h"

#include "Kismet/GameplayStatics.h"

ASupportPackage::ASupportPackage()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASupportPackage::BeginPlay()
{
	Super::BeginPlay();
}

void ASupportPackage::BeginInteraction(ABaseCharacter* Character, APlayerController* PlayerController)
{
	SpawnAircraft(Character, PlayerController);
}

void ASupportPackage::SpawnAircraft(ABaseCharacter* Character, APlayerController* PlayerController)
{
	if (AircraftClass == nullptr) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Aircraft = GetWorld()->SpawnActor<AAircraft>(AircraftClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (Aircraft)
	{
		if (IsControllable)
		{
			//Character->DisableInput(PlayerController);
			//Aircraft->SetPlayerControl(PlayerController);
		}
	}
}

void ASupportPackage::PlayPickupSound()
{
	if (PickupSound == nullptr) return;

	UGameplayStatics::PlaySound2D(GetWorld(), PickupSound);
}

void ASupportPackage::PlayInteractSound()
{
	if (InteractSound == nullptr) return;

	UGameplayStatics::PlaySound2D(GetWorld(), InteractSound);
}

void ASupportPackage::PlayVoiceOverSound(TeamFaction Faction)
{
	for (int i = 0; i < SupportSoundsSet.Num(); i++)
	{
		FSupportPackageVoiceOverSet SupportPackageVoiceOverSet = SupportSoundsSet[i];

		if (SupportPackageVoiceOverSet.Faction == Faction && SupportPackageVoiceOverSet.ReadyToUseSound != nullptr)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), SupportPackageVoiceOverSet.ReadyToUseSound);
			return;
		}
	}
}
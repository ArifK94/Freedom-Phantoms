// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/SupportPackage.h"
#include "Vehicles/Aircraft.h"
#include "Characters/BaseCharacter.h"
#include "CustomComponents/TeamFactionComponent.h"

#include "Kismet/GameplayStatics.h"

ASupportPackage::ASupportPackage()
{
	PrimaryActorTick.bCanEverTick = false;
}

FString ASupportPackage::GetKeyDisplayName_Implementation()
{
	return FString();
}

FString ASupportPackage::OnInteractionFound_Implementation()
{
	return ActionMessage.ToString();
}

void ASupportPackage::OnPickup_Implementation()
{
	SetActorHiddenInGame(true);
	SetHidden(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	PlayPickupSound();

	if (GetOwner())
	{
		auto FactionComp = Cast<UTeamFactionComponent>(GetOwner()->GetComponentByClass(UTeamFactionComponent::StaticClass()));

		if (FactionComp)
		{
			PlayVoiceOverSound(FactionComp->GetSelectedFaction());
		}
	}
}

bool ASupportPackage::OnUseInteraction_Implementation()
{
	PlayInteractSound();

	return true;
}

bool ASupportPackage::CanInteract_Implementation()
{
	if (GetOwner() == nullptr) {
		return true;
	}

	return false;
}

AAircraft* ASupportPackage::SpawnAircraft(ABaseCharacter* Character, APlayerController* PlayerController)
{
	if (AircraftClass == nullptr) {
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAircraft* Aircraft = GetWorld()->SpawnActor<AAircraft>(AircraftClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (Aircraft)
	{
		if (IsControllable)
		{
			//Character->DisableInput(PlayerController);
			//Aircraft->SetPlayerControl(PlayerController);
		}
	}

	return Aircraft;
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
// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/SupportPackage.h"
#include "Vehicles/Aircraft.h"
#include "Characters/BaseCharacter.h"
#include "Controllers/CustomPlayerController.h"
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

FString ASupportPackage::OnInteractionFound_Implementation(APawn* InPawn, AController* InController)
{
	return ActionMessage.ToString();
}

AActor* ASupportPackage::OnPickup_Implementation(APawn* InPawn, AController* InController)
{
	if (!InController) {
		return nullptr;
	}

	auto PlayerController = Cast<ACustomPlayerController>(InController);

	if (!PlayerController) {
		return nullptr;
	}

	PlayerController->AddSupportPackage(this);
	PlayInteractSound();

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

	return this;

}

bool ASupportPackage::OnUseInteraction_Implementation(APawn* InPawn, AController* InController)
{
	if (!InController) {
		return false;
	}

	PlayInteractSound();

	auto PlayerController = Cast<ACustomPlayerController>(InController);

	if (PlayerController)
	{
		AAircraft* Aircraft = SpawnAircraft(Cast<ABaseCharacter>(InPawn), PlayerController);
		PlayerController->SetControlledAircraft(Aircraft, IsControllable);
		return true;
	}

	return false;
}

bool ASupportPackage::CanInteract_Implementation(APawn* InPawn, AController* InController)
{
	if (InController)
	{
		auto Player = Cast<ACustomPlayerController>(InController);

		if (Player)
		{
			return Player->CanAddSupportPackages();
		}
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
	if (SupportSoundsSet.Num() <= 0) {
		return;
	}

	for (int i = 0; i < SupportSoundsSet.Num(); i++)
	{
		FSupportPackageVoiceOverSet SupportPackageVoiceOverSet = SupportSoundsSet[i];

		if (SupportPackageVoiceOverSet.Faction == Faction && SupportPackageVoiceOverSet.ReadyToUseSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), SupportPackageVoiceOverSet.ReadyToUseSound);
			return;
		}
	}
}
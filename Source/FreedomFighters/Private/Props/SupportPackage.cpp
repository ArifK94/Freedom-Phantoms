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

	IsCollected = false;
}

FString ASupportPackage::GetKeyDisplayName_Implementation()
{
	return FString();
}

FString ASupportPackage::OnInteractionFound_Implementation(APawn* InPawn, AController* InController)
{
	return SupportPackageSet.ActionMessage.ToString();
}

AActor* ASupportPackage::OnPickup_Implementation(APawn* InPawn, AController* InController)
{
	if (!InController) {
		return nullptr;
	}

	// Has it already has been picked up?
	if (IsCollected) {
		return nullptr;
	}

	IsCollected = true;

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
		PlayerController->SetControlledAircraft(Aircraft, SupportPackageSet.IsControllable);
		Destroy();
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

void ASupportPackage::BeginPlay()
{
	Super::BeginPlay();

	LoadDataSet();
}

void ASupportPackage::LoadDataSet()
{
	if (SPSetDatatable == nullptr) {
		Destroy(); // no point having this actor exist within the game as it will serve no purpose
		return;
	}

	static const FString ContextString(TEXT("Support Package Set"));
	auto SPSet = SPSetDatatable->FindRow<FSupportPackageSet>(RowName, ContextString, true);

	if (SPSet == nullptr) {
		Destroy(); // no point having this actor exist within the game as it will serve no purpose
		return;
	}


	SupportPackageSet.SupportActorClass = SPSet->SupportActorClass;

	SupportPackageSet.AircraftClass = SPSet->AircraftClass;

	SupportPackageSet.VehicleClass = SPSet->VehicleClass;

	SupportPackageSet.DisplayName = SPSet->DisplayName;

	SupportPackageSet.Description = SPSet->Description;

	SupportPackageSet.ActionMessage  = SPSet->ActionMessage;

	SupportPackageSet.IsControllable = SPSet->IsControllable;

	SupportPackageSet.Icon = SPSet->Icon;

	SupportPackageSet.PreviewImage = SPSet->PreviewImage;

	SupportPackageSet.PickupSound = SPSet->PickupSound;

	SupportPackageSet.InteractSound = SPSet->InteractSound;

	SupportPackageSet.SupportSoundsSet = SPSet->SupportSoundsSet;

	SupportPackageSet.VehiclePathTagName = SPSet->VehiclePathTagName;

}

AAircraft* ASupportPackage::SpawnAircraft(ABaseCharacter* Character, APlayerController* PlayerController)
{
	if (SupportPackageSet.AircraftClass == nullptr) {
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SupportPackageSet.Aircraft = GetWorld()->SpawnActor<AAircraft>(SupportPackageSet.AircraftClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	return SupportPackageSet.Aircraft;
}

void ASupportPackage::PlayPickupSound()
{
	if (SupportPackageSet.PickupSound == nullptr) return;

	UGameplayStatics::PlaySound2D(GetWorld(), SupportPackageSet.PickupSound);
}

void ASupportPackage::PlayInteractSound()
{
	if (SupportPackageSet.InteractSound == nullptr) return;

	UGameplayStatics::PlaySound2D(GetWorld(), SupportPackageSet.InteractSound);
}

void ASupportPackage::PlayVoiceOverSound(TeamFaction Faction)
{
	if (SupportPackageSet.SupportSoundsSet.Num() <= 0) {
		return;
	}

	for (int i = 0; i < SupportPackageSet.SupportSoundsSet.Num(); i++)
	{
		FSupportPackageVoiceOverSet SupportPackageVoiceOverSet = SupportPackageSet.SupportSoundsSet[i];

		if (SupportPackageVoiceOverSet.Faction == Faction && SupportPackageVoiceOverSet.ReadyToUseSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), SupportPackageVoiceOverSet.ReadyToUseSound);
			return;
		}
	}
}
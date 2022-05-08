// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/SupportPackage.h"
#include "Vehicles/VehicleBase.h"
#include "Props/VehicleSplinePath.h"
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

	if (PlayerController) {
		auto Vehicle = SpawnVehicle(Cast<ABaseCharacter>(InPawn), PlayerController);

		if (Vehicle) {
			PlayerController->SetControlledVehicle(Vehicle, SupportPackageSet.IsControllable);
			Destroy();
			return true;
		}
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

AVehicleBase* ASupportPackage::SpawnVehicle(ABaseCharacter* Character, APlayerController* PlayerController)
{
	if (SupportPackageSet.VehicleClass == nullptr) {
		return nullptr;
	}

	auto VehiclePath = AVehicleSplinePath::FindVehiclePath(GetWorld(), SupportPackageSet.VehiclePathTagName);

	// if has a path to follow then spawn the vehicle
	if (VehiclePath) {

		FVector StartLoc = FVector::ZeroVector;
		FRotator StartRot = FRotator::ZeroRotator;

		VehiclePath->GetFirstSplinePoint(StartLoc, StartRot);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SupportPackageSet.Vehicle = GetWorld()->SpawnActor<AVehicleBase>(SupportPackageSet.VehicleClass, StartLoc, StartRot, SpawnParams);

		return SupportPackageSet.Vehicle;
	}

	return nullptr;
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
#include "Props/Interactable.h"
#include "Vehicles/Aircraft.h"
#include "Characters/BaseCharacter.h"

#include "Kismet/GameplayStatics.h"

AInteractable::AInteractable()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AInteractable::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractable::BeginInteraction(ABaseCharacter* Character, APlayerController* PlayerController)
{
	SpawnAircraft(Character, PlayerController);
}

void AInteractable::SpawnAircraft(ABaseCharacter* Character, APlayerController* PlayerController)
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

void AInteractable::PlayPickupSound()
{
	if (PickupSound == nullptr) return;

	UGameplayStatics::PlaySound2D(GetWorld(), PickupSound);
}

void AInteractable::PlayVoiceOverSound(TeamFaction Faction)
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
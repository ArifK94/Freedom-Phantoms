// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "SupportPackage.generated.h"

class AAircraft;
class ABaseCharacter;
class APlayerController;
class UTexture;
class USoundBase;
UCLASS()
class FREEDOMFIGHTERS_API ASupportPackage : public AActor, public IInteractable
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AAircraft> AircraftClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName Description;

	/** Message to be displayed on the UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName ActionMessage;

	/** Is the Spawned Actor Controllable such as aircrafts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsControllable;

	/** Icon displayed for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTexture* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTexture* PreviewImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* PickupSound;

	/** When user beings to interact with the object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* InteractSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FSupportPackageVoiceOverSet> SupportSoundsSet;

public:
	ASupportPackage();

	// Interactable interface methods
	virtual FString GetKeyDisplayName_Implementation() override;
	virtual FString OnInteractionFound_Implementation() override;
	virtual void OnPickup_Implementation() override;
	virtual void OnUseInteraction_Implementation() override;
	virtual bool CanInteract_Implementation() override;

	AAircraft* SpawnAircraft(ABaseCharacter* Character, APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable)
		void PlayPickupSound();

	void PlayInteractSound();

	void PlayVoiceOverSound(TeamFaction Faction);

public:
	FName GetActionMessage() {
		return ActionMessage;
	}

	bool GetIsControllable() {
		return IsControllable;
	}

};

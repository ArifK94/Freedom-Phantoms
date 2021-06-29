// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SupportPackage.generated.h"

class AAircraft;
class ABaseCharacter;
class APlayerController;
class UTexture;
class USoundBase;
UCLASS()
class FREEDOMFIGHTERS_API ASupportPackage : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AAircraft> AircraftClass;
	AAircraft* Aircraft;

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

	void BeginInteraction(ABaseCharacter* Character, APlayerController* PlayerController);

	void SpawnAircraft(ABaseCharacter* Character, APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable)
		void PlayPickupSound();

	void PlayInteractSound();

	void PlayVoiceOverSound(TeamFaction Faction);


protected:
	virtual void BeginPlay() override;

public:
	AAircraft* GetAircraft() {
		return Aircraft;
	}

	FName GetActionMessage() {
		return ActionMessage;
	}

	bool GetIsControllable() {
		return IsControllable;
	}

};

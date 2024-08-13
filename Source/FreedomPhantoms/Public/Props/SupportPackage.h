// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Engine/DataTable.h"
#include "StructCollection.h"
#include "EnumCollection.h"
#include "SupportPackage.generated.h"

class AVehicleBase;
class ABaseCharacter;
class APlayerController;
class UTexture;
class USoundBase;
UCLASS()
class FREEDOMPHANTOMS_API ASupportPackage : public AActor, public IInteractable
{
	GENERATED_BODY()
	
private:
	bool IsCollected;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UDataTable* SPSetDatatable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName RowName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FSupportPackageSet SupportPackageSet;


public:
	ASupportPackage();

	// Interactable interface methods
	virtual FString GetKeyDisplayName_Implementation() override;
	virtual FString OnInteractionFound_Implementation(APawn* InPawn, AController* InController) override;
	virtual AActor* OnPickup_Implementation(APawn* InPawn, AController* InController) override;
	virtual bool OnUseInteraction_Implementation(APawn* InPawn, AController* InController) override;
	virtual bool CanInteract_Implementation(APawn* InPawn, AController* InController) override;

	AVehicleBase* SpawnVehicle(ABaseCharacter* Character, APlayerController* PlayerController);


	UFUNCTION(BlueprintCallable)
		void PlayPickupSound();

	void PlayInteractSound();

	void PlayVoiceOverSound(TeamFaction Faction);

private:
	void LoadDataSet();


protected:
	virtual void BeginPlay() override;

public:
	FSupportPackageSet GetSupportPackageSet() { return SupportPackageSet; }

};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomComponents/HealthComponent.h"
#include "StructCollection.h"
#include "Stronghold.generated.h"

class UBoxComponent;
class ACombatCharacter;
class UFactionManager;
class UAudioComponent;
class AAmmoCrate;
class UCoverPointComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStrongholdCapturedSignature, FOccupiedFaction, OccupiedFaction);


UCLASS()
class FREEDOMFIGHTERS_API AStronghold : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* StrongholdArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* SpawnBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* FactionFlag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* StrongholdAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* CaptureSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* OverrunSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int SpawnMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float SpawnRate;
	FTimerHandle THandler_SpawnDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName SpawnAreaComponentTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* NeutralFlagMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int32 FlagClothMaterialIndex;


private:
	FTimerHandle THandler_OverlappingCombatatant;

	TArray<UBoxComponent*> SpawnAreas;

	TArray<UCoverPointComponent*> CoverPoints;

	FOccupiedFaction* DominantFaction;

	TArray<ACombatCharacter*> TotalOccupyingCombatants;
	TArray<ACombatCharacter*> DefendingCombatatants;

	TArray<FOccupiedFaction*> OccupiedFactions;

public:	
	AStronghold();

	UCoverPointComponent* GetCoverPoint(AActor* OwningCharacter);

	void RemoveDefender(AActor* Actor);

private:
	void CheckOverlappingCombatatant();

	void GetSpawnAreas();

	void LoadCoverPoints();

	void SpawnDefender();
		
	void UpdateTotalOccupants();

	void UpdateDefenders();
	
	void AddFaction(ACombatCharacter* Character, TeamFaction Faction);
	
	FOccupiedFaction* DoesFactionExist(TeamFaction Faction);
	
	bool DoesOccupantExist(ACombatCharacter* Occupant);
	
	void GetHighestFaction();

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnStrongholdCapturedSignature OnStrongholdCaptured;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;



};

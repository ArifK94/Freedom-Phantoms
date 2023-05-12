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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStrongholdDefenderSpawnedSignature, FStrongholdDefender, StrongholdDefender);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStrongholdUnderAttackSignature, bool, IsUnderAttack);


UCLASS()
class FREEDOMPHANTOMS_API AStronghold : public AActor
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
	float DefaultSpawnRate;
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
	
	FOccupiedFaction* AddFaction(ACombatCharacter* Character, TeamFaction Faction);
	
	FOccupiedFaction* GetFaction(TArray<FOccupiedFaction*> Factions, TeamFaction Faction);
		
	void GetHighestFaction();

	/** Does the stronghold have enemy opponents in its area? */
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool IsUnderAttack();
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnStrongholdCapturedSignature OnStrongholdCaptured;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnStrongholdDefenderSpawnedSignature OnStrongholdDefenderSpawned;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnStrongholdUnderAttackSignature OnStrongholdUnderAttack;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;



};

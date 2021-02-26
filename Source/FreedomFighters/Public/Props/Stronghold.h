#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CustomComponents/HealthComponent.h"

#include "Stronghold.generated.h"

class UBoxComponent;
class ACombatCharacter;
class UFactionManager;


// dynamically add the dominant in case more factions are added in the future
USTRUCT(BlueprintType)
struct FOccupiedFaction
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
		int FactionCount;

	UPROPERTY()
		TeamFaction Faction;

	UPROPERTY()
		UMaterialInterface* FlagMaterial;

	UPROPERTY()
		UFactionManager* FactionManager;

	FOccupiedFaction()
	{

	}
};



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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int SpawnMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float SpawnRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float SpawnDelayMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float SpawnDelayMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName SpawnAreaComponentTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* NeutralFlagMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int32 FlagClothMaterialIndex;



private:
	int CurrentSpawnedActors;
	FTimerHandle THandler_SpawnDelay;

	TArray<UBoxComponent*> SpawnAreas;

	FOccupiedFaction* DominantFaction;

	TArray<ACombatCharacter*> OccupyingCharacters;

	TArray<FOccupiedFaction*> OccupiedFactions;

	void AddFaction(ACombatCharacter* Character, TeamFaction Faction);
	
public:	
	AStronghold();

	void StartSpawn();
	void StopSpawn();

private:
	void GetSpawnAreas();
	void SpawnCharacter();
	void UpdateFaction();
	bool DoesFactionExist(TeamFaction Faction);
	bool DoesOccupantExist(ACombatCharacter* Occupant);
	void GetHighestFaction();

	UFUNCTION()
		void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;



};

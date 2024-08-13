#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StructCollection.h"

#include "ObjectPoolComponent.generated.h"

class AGameStateBaseCustom;
class AObjectPoolActor;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	UPROPERTY(Transient)
		TObjectPtr<class UGameInstance>	OwningGameInstance;

	UPROPERTY()
		AGameStateBaseCustom* GameStateBaseCustom;

	UPROPERTY()
		TArray<FObjectPoolParameters> ActorsInObjectPool;

public:	
	UObjectPoolComponent();

	TArray<FObjectPoolParameters> AddToPool(FObjectPoolParameters ObjectPoolParams);

	AObjectPoolActor* ActivatePoolObject(TSubclassOf<AActor> ActorClass, AActor* Owner, FVector const& Location, FRotator const& Rotation);

	// Using Game state list of pool objects rather than itself in order to prevent less object actors to be created such as bullets, don't want a dozen of common projectiles spawned
	AObjectPoolActor* ActivatePoolObject(TSubclassOf<AActor> ActorClass, AActor* Owner, FVector const& Location, FRotator const& Rotation, bool UseGameState);

protected:
	virtual void BeginPlay() override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

public:
	TArray<FObjectPoolParameters> GetActorsInObjectPool() {
		return ActorsInObjectPool;
	}

};

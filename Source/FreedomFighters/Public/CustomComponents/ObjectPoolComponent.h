#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StructCollection.h"

#include "ObjectPoolComponent.generated.h"

class AObjectPoolActor;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		int PoolSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = 0.0f))
		float LifeSpan;

	TArray<FObjectPoolParameters*> ActorsInObjectPool;

public:	
	UObjectPoolComponent();

	TArray<FObjectPoolParameters*> AddToPool(FObjectPoolParameters* ObjectPoolParams);

	void ActivatePoolObject(TSubclassOf<AActor> ActorClass, AActor* Owner, FVector const& Location, FRotator const& Rotation);

protected:
	virtual void BeginPlay() override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

public:
	TArray<FObjectPoolParameters*> GetActorsInObjectPool() {
		return ActorsInObjectPool;
	}

};

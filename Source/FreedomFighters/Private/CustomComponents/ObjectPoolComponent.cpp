#include "CustomComponents/ObjectPoolComponent.h"
#include "ObjectPoolActor.h"
#include "GameFramework/ProjectileMovementComponent.h"

UObjectPoolComponent::UObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UObjectPoolComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UObjectPoolComponent::AddToPool(AActor* Owner, FObjectPoolParameters ObjectPoolParams)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int i = 0; i < ObjectPoolParams.PoolSize; i++)
	{
		AObjectPoolActor* ActorObj = GetWorld()->SpawnActor<AObjectPoolActor>(ObjectPoolParams.PoolableActorClass, FVector().ZeroVector, FRotator().ZeroRotator);

		if (ActorObj)
		{
			// for some reason, the owner needs to be set after spawning the object instead of assigning it from SpawnParams struct.
			ActorObj->SetOwner(Owner);

			// add the pool params to memory
			FObjectPoolParameters* ObjectPoolParameters = new FObjectPoolParameters();
			ObjectPoolParameters->PoolSize = ObjectPoolParams.PoolSize;
			ObjectPoolParameters->PoolableActorClass = ObjectPoolParams.PoolableActorClass;
			ObjectPoolParameters->PoolableActor = ActorObj;

			ActorsInObjectPool.Add(ObjectPoolParameters);
		}
	}
}


AObjectPoolActor* UObjectPoolComponent::ActivatePoolObject(TSubclassOf<AActor> ActorClass, FVector const& Location, FRotator const& Rotation)
{
	for (FObjectPoolParameters* PoolParam : ActorsInObjectPool)
	{
		if (PoolParam->PoolableActorClass == ActorClass)
		{
			AObjectPoolActor* PoolableActor = PoolParam->PoolableActor;
			if (PoolableActor != nullptr && !PoolableActor->IsActive()) 
			{
				PoolableActor->SetActorLocationAndRotation(Location, Rotation.Quaternion());
				PoolableActor->Activate();
				return PoolParam->PoolableActor;
			}
		}
	}
	return nullptr;
}
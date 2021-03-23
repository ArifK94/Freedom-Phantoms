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

void UObjectPoolComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	// the bullets will no longer be needed when parent is destroyed, 
	// for instance, ac130 bullets will not be needed once the aircraft weapons are destroyed
	for (int i = 0; i < ActorsInObjectPool.Num(); i++)
	{
		AObjectPoolActor* PoolableActor = ActorsInObjectPool[i]->PoolableActor;

		if (PoolableActor)
		{
			PoolableActor->Destroy();
		}
	}
}

void UObjectPoolComponent::AddToPool(AActor* Owner, FObjectPoolParameters* ObjectPoolParams)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int i = 0; i < ObjectPoolParams->PoolSize; i++)
	{
		AObjectPoolActor* ActorObj = GetWorld()->SpawnActor<AObjectPoolActor>(ObjectPoolParams->PoolableActorClass, FVector().ZeroVector, FRotator().ZeroRotator);

		if (ActorObj)
		{
			ActorObj->Deactivate();

			// for some reason, the owner needs to be set after spawning the object instead of assigning it from SpawnParams struct.
			ActorObj->SetOwner(Owner);

			// add the pool params to memory
			FObjectPoolParameters* ObjectPoolParameters = new FObjectPoolParameters();
			ObjectPoolParameters->PoolSize = ObjectPoolParams->PoolSize;
			ObjectPoolParameters->PoolableActorClass = ObjectPoolParams->PoolableActorClass;
			ObjectPoolParameters->PoolableActor = ActorObj;

			ActorsInObjectPool.Add(ObjectPoolParameters);
		}
	}
}


void UObjectPoolComponent::ActivatePoolObject(TSubclassOf<AActor> ActorClass, FVector const& Location, FRotator const& Rotation)
{
	// Backup param incase an object cannot be found
	FObjectPoolParameters* ObjectPoolParams = nullptr;
	AActor* Owner = nullptr;

	for (int i = 0; i < ActorsInObjectPool.Num(); i++)
	{
		if (ActorsInObjectPool[i]->PoolableActorClass == ActorClass)
		{
			AObjectPoolActor* PoolableActor = ActorsInObjectPool[i]->PoolableActor;

			if (ObjectPoolParams == nullptr) {
				ObjectPoolParams = new FObjectPoolParameters();
				ObjectPoolParams->PoolSize = 1;
				ObjectPoolParams->PoolableActorClass = ActorClass;

			}
			if (PoolableActor == nullptr)
			{
				// Actors can be destroyed if reached outside of map
				ActorsInObjectPool.RemoveAt(i);
			}
			else
			{
				if (Owner == nullptr) {
					Owner = ActorsInObjectPool[i]->PoolableActor->GetOwner();

				}

				if (PoolableActor != nullptr && !PoolableActor->IsActive())
				{
					PoolableActor->SetActorLocationAndRotation(Location, Rotation.Quaternion());
					PoolableActor->Activate();
					return;
				}
			}



		}
	}


	// Create another Pool Actor if none retrieved from previous loop
	if (ObjectPoolParams)
	{
		AddToPool(Owner, ObjectPoolParams);
		ActivatePoolObject(ObjectPoolParams->PoolableActorClass, Location, Rotation);
	}

}
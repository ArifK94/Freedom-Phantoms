#include "CustomComponents/ObjectPoolComponent.h"
#include "Managers/GameStateBaseCustom.h"
#include "ObjectPoolActor.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UObjectPoolComponent::UObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UObjectPoolComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningGameInstance = Cast<UGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));


	auto GameState = UGameplayStatics::GetGameState(GetWorld());

	if (GameState) {
		GameStateBaseCustom = Cast<AGameStateBaseCustom>(GameState);
	}
}

void UObjectPoolComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	// the bullets will no longer be needed when parent is destroyed, 
	// for instance, ac130 bullets will not be needed once the aircraft weapons are destroyed
	for (int i = 0; i < ActorsInObjectPool.Num(); i++)
	{
		AObjectPoolActor* PoolableActor = ActorsInObjectPool[i].PoolableActor;

		if (UKismetSystemLibrary::IsValid(PoolableActor))
		{
			PoolableActor->Destroy();
		}
	}
}

TArray<FObjectPoolParameters> UObjectPoolComponent::AddToPool(FObjectPoolParameters ObjectPoolParams)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int i = 0; i < ObjectPoolParams.PoolSize; i++)
	{
		AObjectPoolActor* ActorObj = OwningGameInstance->GetWorld()->SpawnActor<AObjectPoolActor>(ObjectPoolParams.PoolableActorClass, FVector().ZeroVector, FRotator().ZeroRotator);

		if (ActorObj)
		{
			ActorObj->Deactivate();

			// add the pool params to memory
			auto ObjectPoolParameters = FObjectPoolParameters();
			ObjectPoolParameters.PoolSize = ObjectPoolParams.PoolSize;
			ObjectPoolParameters.PoolableActorClass = ObjectPoolParams.PoolableActorClass;
			ObjectPoolParameters.PoolableActor = ActorObj;

			ActorsInObjectPool.Add(ObjectPoolParameters);

			if (GameStateBaseCustom) {
				GameStateBaseCustom->AddPoolActor(ObjectPoolParameters);
			}
		}
	}
	return ActorsInObjectPool;
}


AObjectPoolActor* UObjectPoolComponent::ActivatePoolObject(TSubclassOf<AActor> ActorClass, AActor* Owner, FVector const& Location, FRotator const& Rotation)
{
	// Backup param incase an object cannot be found
	AObjectPoolActor* PoolableActor = nullptr;

	for (int i = ActorsInObjectPool.Num() - 1; i >= 0; i--)
	{
		if (ActorsInObjectPool[i].PoolableActorClass == ActorClass)
		{
			PoolableActor = ActorsInObjectPool[i].PoolableActor;

			if (PoolableActor == nullptr || !UKismetSystemLibrary::IsValid(PoolableActor))
			{
				// Actors can be destroyed if reached outside of map
				ActorsInObjectPool.RemoveAt(i);
			}
			else
			{
				if (UKismetSystemLibrary::IsValid(PoolableActor) && !PoolableActor->IsActive())
				{
					ActorsInObjectPool[i].PoolableActor->SetOwner(Owner);
					PoolableActor->SetActorLocationAndRotation(Location, Rotation.Quaternion());
					PoolableActor->Activate();
					return PoolableActor;
				}
			}
		}
	}


	// Create another Pool Actor if none retrieved
	auto ObjectPoolParams = FObjectPoolParameters();
	ObjectPoolParams.PoolSize = 1;
	ObjectPoolParams.PoolableActorClass = ActorClass;
	AddToPool(ObjectPoolParams);
	PoolableActor = ActivatePoolObject(ObjectPoolParams.PoolableActorClass, Owner, Location, Rotation);


	return PoolableActor;
}

AObjectPoolActor* UObjectPoolComponent::ActivatePoolObject(TSubclassOf<AActor> ActorClass, AActor* Owner, FVector const& Location, FRotator const& Rotation, bool UseGameState)
{
	if (UseGameState && GameStateBaseCustom)
	{
		auto Actor = GameStateBaseCustom->GetAvailablePoolActor(ActorClass);

		if (UKismetSystemLibrary::IsValid(Actor))
		{
			Actor->SetOwner(Owner);
			Actor->SetActorLocationAndRotation(Location, Rotation.Quaternion());
			Actor->Activate();
			return Actor;
		}
		else
		{
			return ActivatePoolObject(ActorClass, Owner, Location, Rotation);
		}
	}
	else
	{
		return ActivatePoolObject(ActorClass, Owner, Location, Rotation);
	}

}
#include "CustomComponents/ObjectPoolComponent.h"

#include "GameFramework/ProjectileMovementComponent.h"

UObjectPoolComponent::UObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UObjectPoolComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UObjectPoolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UObjectPoolComponent::EnableActor(AActor* Actor, bool IsEnabled)
{
	Actor->SetActorHiddenInGame(!IsEnabled);
	Actor->SetHidden(!IsEnabled);

	Actor->SetActorEnableCollision(IsEnabled);
	Actor->SetActorTickEnabled(IsEnabled);



	// Projectile Movement Component needs to be updated otherwise when reactivated, the actor pool object will have be stationary.
	// This was the problem for bullets not moving again after being reactivated.
	// However, Projectile Movement Component does not have a reset function when trying to reactivate so creating new Projectile Movement Component with same settings seems to work
	// After creating / registering the new Projectile Movement Component, delete the previous Projectile Movement Component
	if (IsEnabled)
	{
		UProjectileMovementComponent* ProjectileMovementComp = Cast<UProjectileMovementComponent>(Actor->GetComponentByClass(UProjectileMovementComponent::StaticClass()));

		if (ProjectileMovementComp)
		{
			UProjectileMovementComponent* NewProjectileMovementComp = NewObject<UProjectileMovementComponent>(Actor);
			NewProjectileMovementComp->InitialSpeed = ProjectileMovementComp->InitialSpeed;
			NewProjectileMovementComp->MaxSpeed = ProjectileMovementComp->MaxSpeed;
			NewProjectileMovementComp->Bounciness = ProjectileMovementComp->Bounciness;
			NewProjectileMovementComp->Friction = ProjectileMovementComp->Friction;
			NewProjectileMovementComp->ProjectileGravityScale = ProjectileMovementComp->ProjectileGravityScale;

			// delete it before registering new Projectile Movement Component in case any physics are applied and cause a clash between two of the componentss
			ProjectileMovementComp->DestroyComponent();
			NewProjectileMovementComp->RegisterComponent();
		}
		else
		{
			ProjectileMovementComp->Deactivate();
		}
	}
}


void UObjectPoolComponent::AddToPool(AActor* Owner, FObjectPoolParameters ObjectPoolParams)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int i = 0; i < ObjectPoolParams.PoolSize; i++)
	{
		AActor* ActorObj = GetWorld()->SpawnActor<AActor>(ObjectPoolParams.ActorClass, FVector().ZeroVector, FRotator().ZeroRotator);

		if (ActorObj)
		{
			// for some reason, the owner needs to be set after spawning the object instead of assigning it from SpawnParams struct.
			ActorObj->SetOwner(Owner);

			// add the pool params to memory
			FObjectPoolParameters* ObjectPoolParameters = new FObjectPoolParameters();
			ObjectPoolParameters->PoolSize = ObjectPoolParams.PoolSize;
			ObjectPoolParameters->LifeSpan = ObjectPoolParams.LifeSpan;
			ObjectPoolParameters->ActorClass = ObjectPoolParams.ActorClass;
			ObjectPoolParameters->Actor = ActorObj;
			ObjectPoolParameters->IsEnabled = false;

			// UProjectileMovementComponent reset does not exist so we deactivate existing component so we can create a new UProjectileMovementComponent with same settings
			UProjectileMovementComponent* ProjectileMovementComp = Cast<UProjectileMovementComponent>(ActorObj->GetComponentByClass(UProjectileMovementComponent::StaticClass()));
			if (ProjectileMovementComp)
			{
				ProjectileMovementComp->Deactivate();
			}
			EnableActor(ActorObj, false);

			ActorsInObjectPool.Add(ObjectPoolParameters);
		}
	}


}

void UObjectPoolComponent::ActivatePoolObject(TSubclassOf<AActor> ActorClass, FVector const& Location, FRotator const& Rotation, AActor* Owner, FObjectPoolParameters ObjectPoolParams)
{
	bool HasGotPoolObject = false;
	for (FObjectPoolParameters* ObjectPoolUnit : ActorsInObjectPool)
	{
		// if the class has been found
		if (ObjectPoolUnit->ActorClass == ActorClass)
		{
			AActor* Actor = ObjectPoolUnit->Actor;

			if (!ObjectPoolUnit->IsEnabled)
			{
				Actor->SetActorLocationAndRotation(Location, Rotation.Quaternion());
				EnableActor(Actor, true);
				ObjectPoolUnit->IsEnabled = true;

				//Binding the function with specific values
				ObjectPoolUnit->TimerDel.BindUFunction(this, FName("DeactivatePoolObject"), Actor);
				GetWorld()->GetTimerManager().SetTimer(ObjectPoolUnit->THandler_LifeSpan, ObjectPoolUnit->TimerDel, ObjectPoolUnit->LifeSpan, false);
				
				HasGotPoolObject = true;
				break;
			}
		}
	}


	if (HasGotPoolObject) {
		return;
	}


	// if not found one, then spawn another
	if (Owner)
	{
		AddToPool(Owner, ObjectPoolParams);
	}



}


void UObjectPoolComponent::DeactivatePoolObject(AActor* Actor)
{
	for (FObjectPoolParameters* ObjectPoolUnit : ActorsInObjectPool)
	{
		// if the class has been found
		if (ObjectPoolUnit->Actor == Actor)
		{
			AActor* Actor = ObjectPoolUnit->Actor;
			EnableActor(Actor, false);
			ObjectPoolUnit->IsEnabled = false;
			Actor->GetWorldTimerManager().ClearTimer(ObjectPoolUnit->THandler_LifeSpan);
			break;
		}
	}
}
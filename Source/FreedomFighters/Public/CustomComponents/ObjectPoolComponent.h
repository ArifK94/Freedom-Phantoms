#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectPoolComponent.generated.h"

USTRUCT(BlueprintType)
struct FREEDOMFIGHTERS_API FObjectPoolParameters
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
		TSubclassOf<AActor> ActorClass;

	UPROPERTY()
		AActor* Actor;

	UPROPERTY()
		int PoolSize;

	UPROPERTY()
		float LifeSpan;

	UPROPERTY()
		bool IsEnabled;

	FTimerHandle THandler_LifeSpan;
	FTimerDelegate TimerDel;
};


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

	void AddToPool(AActor* Owner, FObjectPoolParameters ObjectPoolParams);

	/** TSubclassOf is given as this will be used to compare which object needs to be called to activate */
	void ActivatePoolObject(TSubclassOf<AActor> ActorClass, FVector const& Location, FRotator const& Rotation, AActor* Owner = nullptr, FObjectPoolParameters ObjectPoolParams = FObjectPoolParameters());


	/** UFUNCTION() needed to make the timer delegate to work */
	UFUNCTION()
		void DeactivatePoolObject(AActor* Actor);

private:
	void EnableActor(AActor* Actor, bool IsEnabled);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};

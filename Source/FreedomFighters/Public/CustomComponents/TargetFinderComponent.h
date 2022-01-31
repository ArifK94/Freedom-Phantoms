// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetFinderComponent.generated.h"

class USphereComponent;
class UAISenseConfig;
class UAISenseConfig_Sight;
class UAISense;
class UAIPerceptionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetSearchSignature, AActor*, TargetActor);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UTargetFinderComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	USphereComponent* TargetSightSphere;
	UAISenseConfig_Sight* AISightConfig;
	UAIPerceptionComponent* PerceptionComp;

	FTimerHandle THandler_TargetSearch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool FindTargetPerFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TargetSightRadius;

	/** Limit the number of targets the component can process */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int FinderLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool ShowDebugTrace;

	/** Type of actors to accept. Empty list will return all actor classes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<AActor>> ClassFilters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<TEnumAsByte<EObjectTypeQuery>> CollisionChannels;

public:	
	UTargetFinderComponent();

	void Init();

	UFUNCTION(BlueprintCallable)
		AActor* FindTarget();

	bool IsTargetBehind(AActor* ActorA, AActor* TargetActor);

	//UAISenseConfig* GetPerceptionSenseConfig(TSubclassOf<UAISense> SenseClass);

	//void SetVisionAngle();

private:
	void FindTargetUpdate();

	bool GetTrace(FHitResult& OutHit, FVector Start, FVector End);

	bool IsActorFiltered(AActor* Actor);

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnTargetSearchSignature OnTargetSearch;

	void SetFindTargetPerFrame(bool Value) { FindTargetPerFrame = Value; }

	void AddClassFilter(TSubclassOf<AActor> Class) { ClassFilters.Add(Class); }
		
};

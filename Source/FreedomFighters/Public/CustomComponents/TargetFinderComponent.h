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

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnTargetSearchSignature OnTargetSearch;

	bool SetFindTargetPerFrame(bool Value) { FindTargetPerFrame = Value; }
		
};

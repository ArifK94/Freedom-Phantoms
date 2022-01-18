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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UTargetFinderComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	USphereComponent* TargetSightSphere;
	UAISenseConfig_Sight* AISightConfig;
	UAIPerceptionComponent* PerceptionComp;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TargetSightRadius;

	/** Limit the number of targets the component can process */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int FinderLimit;

public:	
	UTargetFinderComponent();

	void Init();

	AActor* FindTarget();

	bool IsTargetBehind(AActor* ActorA, AActor* TargetActor);

	//UAISenseConfig* GetPerceptionSenseConfig(TSubclassOf<UAISense> SenseClass);

	//void SetVisionAngle();

protected:
	virtual void BeginPlay() override;	

		
};

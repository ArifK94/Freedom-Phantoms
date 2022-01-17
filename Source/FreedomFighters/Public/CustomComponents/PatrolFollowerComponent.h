// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PatrolFollowerComponent.generated.h"


class APatrolPath;

class USplineComponent;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UPatrolFollowerComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	USplineComponent* SplinePathComp;

	int CurrentPathIndex;

	// Patrol points in ASCENDING order
	bool IsPatrolASC;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		APatrolPath* PatrolPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName PathTagName;

public:	
	UPatrolFollowerComponent();

	void GetCurrentPathPoint(FVector& OutLocation);
	void GetNextPathPoint(FVector& OutLocation);

protected:
	virtual void BeginPlay() override;


		
};

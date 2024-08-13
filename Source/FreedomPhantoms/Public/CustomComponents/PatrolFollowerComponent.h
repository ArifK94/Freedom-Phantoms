// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PatrolFollowerComponent.generated.h"


class APatrolPath;

class USplineComponent;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UPatrolFollowerComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	UPROPERTY()
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

	FVector GetCurrentPathPoint();
	FVector GetNextPathPoint();

protected:
	virtual void BeginPlay() override;


public:

	APatrolPath* GetPatrolPath() { return PatrolPath; }
		
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolPath.generated.h"

class USplineComponent;
UCLASS()
class FREEDOMPHANTOMS_API APatrolPath : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USplineComponent* SplinePathComp;
	
public:	
	APatrolPath();

protected:
	virtual void BeginPlay() override;

public:
	USplineComponent* GetSplinePathComp() { return SplinePathComp; }

};

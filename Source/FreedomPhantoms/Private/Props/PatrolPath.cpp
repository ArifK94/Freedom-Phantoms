// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/PatrolPath.h"

#include "Components/SplineComponent.h"

APatrolPath::APatrolPath()
{
	PrimaryActorTick.bCanEverTick = false;

	SplinePathComp = CreateDefaultSubobject<USplineComponent>("SplinePathComp");
}

void APatrolPath::BeginPlay()
{
	Super::BeginPlay();
	
}


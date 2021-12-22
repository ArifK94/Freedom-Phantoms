// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/AIMovementComponent.h"

UAIMovementComponent::UAIMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UAIMovementComponent::BeginPlay()
{
	Super::BeginPlay();	
}

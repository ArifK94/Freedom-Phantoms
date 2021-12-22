// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/TargetPointComponent.h"

UTargetPointComponent::UTargetPointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UTargetPointComponent::BeginPlay()
{
	Super::BeginPlay();	
}


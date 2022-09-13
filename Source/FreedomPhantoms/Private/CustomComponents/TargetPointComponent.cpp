// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/TargetPointComponent.h"
#include "CustomComponents/HealthComponent.h"

UTargetPointComponent::UTargetPointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UTargetPointComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UTargetPointComponent::OnHealthChanged(FHealthParameters InHealthParameters)
{
	if (!InHealthParameters.AffectedHealthComponent->IsAlive())
	{
		HealthComp->OnHealthChanged.RemoveDynamic(this, &UTargetPointComponent::OnHealthChanged);
		IsPointTaken = false;
	}
}

void UTargetPointComponent::SetPoint(AActor* Owner)
{
	if (!Owner) {
		return;
	}

	HealthComp = Cast<UHealthComponent>(Owner->GetComponentByClass(UHealthComponent::StaticClass()));

	if (HealthComp)
	{
		HealthComp->OnHealthChanged.AddDynamic(this, &UTargetPointComponent::OnHealthChanged);
		IsPointTaken = true;
	}
}
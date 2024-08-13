// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/TargetPointComponent.h"
#include "CustomComponents/HealthComponent.h"

#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"

UTargetPointComponent::UTargetPointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetCollisionProfileName(TEXT("NoCollision"));
	SphereComponent->SetupAttachment(GetAttachmentRoot());
	SphereComponent->ShapeColor = FColor(0, 0, 255, 255);

	FLinearColor ArrowColour = FLinearColor();
	ArrowColour.R = 255.0f;
	ArrowColour.G = 255.0f;
	ArrowColour.B = 0.0f;
	ArrowColour.A = 1.0f;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetCollisionProfileName(TEXT("NoCollision"));
	ArrowComponent->SetupAttachment(GetAttachmentRoot());
	ArrowComponent->SetArrowColor(ArrowColour);
}

void UTargetPointComponent::OnHealthChanged(FHealthParameters InHealthParameters)
{
	if (!InHealthParameters.AffectedHealthComponent->IsAlive())
	{
		HealthComp->OnHealthChanged.RemoveDynamic(this, &UTargetPointComponent::OnHealthChanged);
		CurrentOwner = nullptr;
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
		CurrentOwner = Owner;
		HealthComp->OnHealthChanged.AddDynamic(this, &UTargetPointComponent::OnHealthChanged);
		IsPointTaken = true;
	}
}
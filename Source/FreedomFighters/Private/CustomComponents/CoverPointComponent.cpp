#include "CustomComponents/CoverPointComponent.h"

#include "Components/ArrowComponent.h"

UCoverPointComponent::UCoverPointComponent()
{
	SetCollisionProfileName(TEXT("NoCollision"));
}


void UCoverPointComponent::BeginPlay()
{
	Super::BeginPlay();	
}


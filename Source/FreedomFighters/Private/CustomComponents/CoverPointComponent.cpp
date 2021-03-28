#include "CustomComponents/CoverPointComponent.h"

#include "Components/ArrowComponent.h"

UCoverPointComponent::UCoverPointComponent()
{
	SetCollisionProfileName(TEXT("NoCollision"));
	ShapeColor = FColor(255, 255, 0, 255);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetCollisionProfileName(TEXT("NoCollision"));
	ArrowComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);


	FLinearColor ArrowColour = FLinearColor();
	ArrowColour.R = 255.0f;
	ArrowColour.G = 255.0f;
	ArrowColour.B = 0.0f;
	ArrowColour.A = 1.0f;

	ArrowComponent->SetArrowColor(ArrowColour);

	IsAtCornerLeft = false;
	IsAtCornerRight = false;
}
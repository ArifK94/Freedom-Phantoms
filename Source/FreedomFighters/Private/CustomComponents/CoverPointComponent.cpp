#include "CustomComponents/CoverPointComponent.h"

#include "Components/ArrowComponent.h"

UCoverPointComponent::UCoverPointComponent()
{
	SetCollisionProfileName(TEXT("NoCollision"));
	ShapeColor = FColor(255, 255, 0, 255);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetCollisionProfileName(TEXT("NoCollision"));
	ArrowComponent->SetupAttachment(GetAttachmentRoot());


	FLinearColor ArrowColour = FLinearColor();
	ArrowColour.R = 255.0f;
	ArrowColour.G = 255.0f;
	ArrowColour.B = 0.0f;
	ArrowColour.A = 1.0f;

	ArrowComponent->SetArrowColor(ArrowColour);

	IsAtCornerLeft = false;
	IsAtCornerRight = false;

	IsPriority = false;

	UpdateShapes();
}

#if WITH_EDITOR
void UCoverPointComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdateShapes();
}
#endif

void UCoverPointComponent::UpdateShapes()
{
	FLinearColor ArrowColour = FLinearColor();

	if (IsPriority) {
		ArrowColour.R = 255.0f;
		ArrowColour.G = 0.0f;
		ArrowColour.B = 0.0f;
		ArrowColour.A = 1.0f;
		ShapeColor = FColor(255, 0, 0, 255);
	}
	else {
		ArrowColour.R = 255.0f;
		ArrowColour.G = 255.0f;
		ArrowColour.B = 0.0f;
		ArrowColour.A = 1.0f;
		ShapeColor = FColor(255, 255, 0, 255);
	}

	ArrowComponent->SetArrowColor(ArrowColour);
}
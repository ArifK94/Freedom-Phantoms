#include "Props/TargetSystemMarker.h"

#include "Components/WidgetComponent.h"

ATargetSystemMarker::ATargetSystemMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	MarkerComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("MarkerComponent"));
	MarkerComponent->SetWidgetSpace(EWidgetSpace::Screen);
	RootComponent = MarkerComponent;
}
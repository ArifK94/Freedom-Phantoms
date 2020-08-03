

#include "Props/BaseCoverProp.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"

ABaseCoverProp::ABaseCoverProp()
{
	CoverArea = CreateDefaultSubobject<UBoxComponent>(TEXT("CoverArea"));
	RootComponent = CoverArea;
	CoverArea->SetCollisionProfileName(TEXT("OverlapAll"));

	ForwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardDirection"));
	ForwardDirection->AttachTo(CoverArea);
}
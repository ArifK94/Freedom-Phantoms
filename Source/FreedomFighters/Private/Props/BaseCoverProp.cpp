

#include "Props/BaseCoverProp.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"

ABaseCoverProp::ABaseCoverProp()
{
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	CoverArea = CreateDefaultSubobject<UBoxComponent>(TEXT("CoverArea"));
	CoverArea->AttachTo(Root);
	CoverArea->SetCollisionProfileName(TEXT("OverlapAll"));

	ForwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardDirection"));
	ForwardDirection->AttachTo(Root);

	IsCoverHigh = true;
}
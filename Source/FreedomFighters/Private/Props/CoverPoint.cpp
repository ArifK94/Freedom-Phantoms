#include "Props/CoverPoint.h"

ACoverPoint::ACoverPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	ConvertToWorldSpace = false;
}

void ACoverPoint::BeginPlay()
{
	Super::BeginPlay();

	if (ConvertToWorldSpace)
	{

	}
}

void ACoverPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


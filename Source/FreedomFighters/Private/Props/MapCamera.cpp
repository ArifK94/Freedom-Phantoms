#include "Props/MapCamera.h"

AMapCamera::AMapCamera()
{
	PrimaryActorTick.bCanEverTick = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	RootComponent = Camera;

	PostProcessor = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessor"));
}

void AMapCamera::BeginPlay()
{
	Super::BeginPlay();	
}


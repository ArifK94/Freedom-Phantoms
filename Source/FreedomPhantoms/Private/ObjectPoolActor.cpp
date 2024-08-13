#include "ObjectPoolActor.h"

AObjectPoolActor::AObjectPoolActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Lifespan = 5.0f;
}

void AObjectPoolActor::BeginPlay()
{
	Super::BeginPlay();
}

void AObjectPoolActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AObjectPoolActor::IsActive()
{
	return isActive;
}

void AObjectPoolActor::SetActive(bool InpActive)
{
	isActive = InpActive;

	SetActorHiddenInGame(!InpActive);
	SetHidden(!InpActive);
	SetActorEnableCollision(InpActive);
	SetActorTickEnabled(InpActive);

	OnPoolActorToggle.Broadcast(this, InpActive);
}

void AObjectPoolActor::Activate()
{
	SetActive(true);
	GetWorldTimerManager().SetTimer(THandler_LifespanTimer, this, &AObjectPoolActor::Deactivate, Lifespan, false);
}


void AObjectPoolActor::Deactivate() 
{
	SetActive(false);
	GetWorldTimerManager().ClearTimer(THandler_LifespanTimer);
}

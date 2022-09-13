#include "Accessories/Goggle.h"

#include "Components/StaticMeshComponent.h"

AGoggle::AGoggle()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	Mesh->CanCharacterStepUpOn = ECB_No;
}

void AGoggle::BeginPlay()
{
	Super::BeginPlay();
}


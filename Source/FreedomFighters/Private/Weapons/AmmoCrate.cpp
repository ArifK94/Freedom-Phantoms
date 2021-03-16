#include "Weapons/AmmoCrate.h"

#include "Components/AudioComponent.h"

AAmmoCrate::AAmmoCrate()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
	RootComponent = Mesh;

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
}

void AAmmoCrate::PlaySuccess()
{
	if (ReplenishSuccessSound)
	{
		AudioComponent->Sound = ReplenishSuccessSound;
		AudioComponent->Play();		
	}
}

void AAmmoCrate::PlayFailed()
{
	if (ReplenishFailedSound)
	{
		AudioComponent->Sound = ReplenishFailedSound;
		AudioComponent->Play();
	}
}
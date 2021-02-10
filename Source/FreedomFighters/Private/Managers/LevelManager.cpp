#include "Managers/LevelManager.h"

ALevelManager::ALevelManager()
{
	PrimaryActorTick.bCanEverTick = false;

	IsNightTime = false;
}

void ALevelManager::BeginPlay()
{
	Super::BeginPlay();

	ToggleObjects();
}

void ALevelManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelManager::ToggleObjects()
{
	if (DayTimeObjects.Num() > 0)
	{
		for (AActor* Actor : DayTimeObjects)
		{
			if (Actor != nullptr)
			{
				Actor->SetHidden(IsNightTime);

				// Hides visible components
				Actor->SetActorHiddenInGame(IsNightTime);


				TArray<UPrimitiveComponent*> AllComponents;
				Actor->GetComponents<UPrimitiveComponent>(AllComponents);

				for (UPrimitiveComponent* Component : AllComponents)
				{
					Component->SetHiddenInGame(IsNightTime);
				}

			}
		}
	}

	if (NightTimeObjects.Num() > 0)
	{
		for (AActor* Actor : NightTimeObjects)
		{
			if (Actor != nullptr)
			{
				Actor->SetHidden(!IsNightTime);

				// Hides visible components
				Actor->SetActorHiddenInGame(!IsNightTime);


				TArray<UPrimitiveComponent*> AllComponents;
				Actor->GetComponents<UPrimitiveComponent>(AllComponents);

				for (UPrimitiveComponent* Component : AllComponents)
				{
					Component->SetHiddenInGame(!IsNightTime);
				}
			}

		}
	}

}

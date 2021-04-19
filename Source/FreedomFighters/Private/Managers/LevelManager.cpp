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

void ALevelManager::ToggleObjects()
{
	if (DayTimeObjects.Num() > 0)
	{
		for (AActor* Actor : DayTimeObjects)
		{
			if (Actor != nullptr)
			{
				// Hides visible components
				Actor->SetActorHiddenInGame(IsNightTime);
				Actor->SetHidden(IsNightTime);
				Actor->SetActorEnableCollision(!IsNightTime);
				Actor->SetActorTickEnabled(!IsNightTime);
			}
		}
	}

	if (NightTimeObjects.Num() > 0)
	{
		for (AActor* Actor : NightTimeObjects)
		{
			if (Actor != nullptr)
			{
				Actor->SetActorHiddenInGame(!IsNightTime);
				Actor->SetHidden(!IsNightTime);
				Actor->SetActorEnableCollision(IsNightTime);
			}
		}
	}
}
#include "Managers/GameModeManager.h"

#include "Kismet/KismetMathLibrary.h"

bool AGameModeManager::IsCoverPointTaken(FWorldCoverPoint CoverLocation)
{
	if (CoverPoints.Num() > 0)
	{
		for (int i = 0; i < CoverPoints.Num(); i++)
		{
			FVector CoverPoint = CoverPoints[i].Location;

			if (CoverLocation.Owner != CoverPoints[i].Owner && UKismetMathLibrary::EqualEqual_VectorVector(CoverPoint, CoverLocation.Location, 20.0f)) {
				return true;
			}
		}
	}

	return false;
}

void AGameModeManager::AddCoverPoint(FWorldCoverPoint CoverLocation)
{
	CoverPoints.Add(CoverLocation);
}

void AGameModeManager::RemoveCoverPoint(FWorldCoverPoint CoverLocation)
{
	if (CoverPoints.Num() > 0)
	{
		for (int i = 0; i < CoverPoints.Num(); i++)
		{
			FVector CoverPoint = CoverPoints[i].Location;

			if (CoverLocation.Owner == CoverPoints[i].Owner && UKismetMathLibrary::EqualEqual_VectorVector(CoverPoint, CoverLocation.Location)) {
				CoverPoints.RemoveAt(i);
			}
		}
	}
}
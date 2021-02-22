#include "Managers/GameModeManager.h"

#include "Kismet/KismetMathLibrary.h"

bool AGameModeManager::IsCoverPointTaken(FWorldCoverPoint CoverLocation)
{

	if (CoverPoints.Num() > 0)
	{
		int DuplicateAmount = -1;
		ACharacter* PlayerCharacter = nullptr;

		for (int i = 0; i < CoverPoints.Num(); i++)
		{
			FVector CoverPoint = CoverPoints[i].Location;

			if (UKismetMathLibrary::EqualEqual_VectorVector(CoverPoint, CoverLocation.Location, 2.0f))
			{
				DuplicateAmount++;
			}

			if (DuplicateAmount > 0)
			{
				return true;
			}


			if (CoverLocation.Owner != CoverPoints[i].Owner && UKismetMathLibrary::EqualEqual_VectorVector(CoverPoint, CoverLocation.Location, 15.0f)) 
			{
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
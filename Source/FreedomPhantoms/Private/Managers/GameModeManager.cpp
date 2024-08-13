#include "Managers/GameModeManager.h"
#include "Managers/LevelManager.h"
#include "CustomComponents/HealthComponent.h"
#include "Weapons/Weapon.h"
#include "Services/SharedService.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

void AGameModeManager::BeginPlay()
{
	Super::BeginPlay();

	FindLevelManager();
}

void AGameModeManager::FindLevelManager()
{
	AActor* LevelManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), ALevelManager::StaticClass());
	LevelManager = Cast<ALevelManager>(LevelManagerActor);
}


bool AGameModeManager::IsCoverPointTaken(FWorldCoverPoint CoverLocation)
{
	if (CoverPoints.Num() > 0)
	{
		int DuplicateAmount = -1;
		ACharacter* PlayerCharacter = nullptr;

		for (int i = 0; i < CoverPoints.Num(); i++)
		{
			FVector CoverPoint = CoverPoints[i].Location;

			// is there a cover location close to this point?
			if (USharedService::IsNearTargetPosition(CoverPoint, CoverLocation.Location, 10.f))
			{
				if (CoverPoints[i].Owner)
				{
					// is the cover point assigned to a different?
					if (CoverLocation.Owner != CoverPoints[i].Owner)
					{
						// Owner is no longer alive?
						if (!UHealthComponent::IsActorAlive(CoverPoints[i].Owner)) {

							// update the owner to this new owner.
							CoverPoints[i].Owner = CoverLocation.Owner;
							return false;
						}

						// if existing owner is still alive then point is taken.
						return true;
					}
				}
				// if owner is null
				else
				{
					// update the owner to this new owner.
					CoverPoints[i].Owner = CoverLocation.Owner;
					return false;
				}
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
				break;
			}
		}
	}
}

void AGameModeManager::AddDroppedWeapon(AWeapon* Weapon)
{
	Weapon->SetOwner(nullptr);
	DroppedWeapons.Add(Weapon);

	// if dropped list has reached x amoun then remove the first weapon
	if (DroppedWeapons.Num() > 10) {

		if (DroppedWeapons[0]) 
		{
			// weapon can be picked up by player after it has been dropped so need to check if it still has no owner
			if (DroppedWeapons[0]->GetOwner() == nullptr)
			{
				DroppedWeapons[0]->Destroy();
			}

			DroppedWeapons.RemoveAt(0);
		}
		// if weapon returned is null then remove it from the list.
		else
		{
			DroppedWeapons.RemoveAt(0);
		}
	}
}
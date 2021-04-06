#include "Vehicles/AircraftTransport.h"
#include "CustomComponents/HealthComponent.h"
#include "Characters/BaseCharacter.h"
#include "Props/AircraftSplinePath.h"
#include "Accessories/Rope.h"

#include "Kismet/KismetMathLibrary.h"

void AAircraftTransport::BeginPlay()
{
	Super::BeginPlay();

	//GetWorldTimerManager().SetTimer(THandler_Rapelling, this, &AAircraftTransport::WaitForRapelling, 1.0f, true);
}


void AAircraftTransport::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	WaitForRapelling();

	UpdateOccupiedSeats();
}


void AAircraftTransport::WaitForRapelling()
{
	if (CurrentAircraftMovement == EAircraftMovement::Hovering && OccupiedSeats.Num() > 0)
	{
		CurveTimeline.Stop();

		if (RopeClass != nullptr)
		{
			if (RopeLeft == nullptr)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				RopeLeft = GetWorld()->SpawnActor<ARope>(RopeClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				RopeLeft->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftRopeSocket);
			}

			if (RopeRight == nullptr)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				RopeRight = GetWorld()->SpawnActor<ARope>(RopeClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				RopeRight->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightRopeSocket);
			}

		}


		for (int i = 0; i < OccupiedSeats.Num(); i++)
		{
			if (OccupiedSeats[i].Role == EAircraftRole::SideGunner)
			{
				FAircraftSeating Passenger = OccupiedSeats[i];

				if (!Passenger.CharacterObj->IsInHelicopter()) {
					OccupiedSeats.RemoveAt(i);
				}
				else
				{
					if (!Passenger.CharacterObj->IsRepellingDown())
					{
						if (Passenger.isRopeLeftSide)
						{
							if (!isLeftRappelOccupied)
							{
								Passenger.CharacterObj->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftRopeSocket);
								Passenger.CharacterObj->SetIsRepellingDown(true);
								isLeftRappelOccupied = true;
							}
						}
						else
						{
							if (!isRightRappelOccupied)
							{
								Passenger.CharacterObj->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightRopeSocket);
								Passenger.CharacterObj->SetIsRepellingDown(true);
								isRightRappelOccupied = true;
							}
						}
					}

				}
			}
		}
	}

	if (OccupiedSeats.Num() <= 0)
	{
		if (RopeLeft) {
			RopeLeft->DropRope();
		}

		if (RopeRight) {
			RopeRight->DropRope();
		}

		CurveTimeline.Play();

		CurrentAircraftMovement = EAircraftMovement::MovingForward;
	}
}

void AAircraftTransport::UpdateOccupiedSeats()
{
	// check if passengers still alive
	for (int i = 0; i < OccupiedSeats.Num(); i++)
	{
		if (OccupiedSeats[i].Role == EAircraftRole::SideGunner)
		{
			FAircraftSeating Passenger = OccupiedSeats[i];
			if (Passenger.CharacterObj) {

				UHealthComponent* CurrentHealth = Cast<UHealthComponent>(Passenger.CharacterObj->GetComponentByClass(UHealthComponent::StaticClass()));

				if (!CurrentHealth->IsAlive())
				{
					OccupiedSeats.RemoveAt(i);
				}
			}
		}
	}
}
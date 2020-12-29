// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/CombatAIController.h"

#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/Shotgun.h"

#include "Kismet/KismetMathLibrary.h"

#include "CustomComponents/HealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/EngineTypes.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "..\..\Public\Controllers\CombatAIController.h"

ACombatAIController::ACombatAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	AcceptanceRadius = 100.0f;
	DistanceDiffSprint = 200.0f;
}

void ACombatAIController::Init()
{
	if (OwningCombatCharacter) {
		UpdateCharacterMovement();

		PerceptionComp = Cast<UAIPerceptionComponent>(OwningCombatCharacter->GetComponentByClass(UAIPerceptionComponent::StaticClass()));
	}
}

void ACombatAIController::BeginPlay()
{
	Super::BeginPlay();

	OwningCombatCharacter = Cast<ACombatCharacter>(AController::GetPawn());

	Init();

	CurrentDeltaTime = 0.0f;
	BulletFireCountDown = 0.0f;
	FiringWaitTime = FMath::RandRange(1.0f, 3.0f);

	// run behavior tree if specified
	if (BTAsset) {
		AAIController::RunBehaviorTree(BTAsset);
	}
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get owning character
	OwningCombatCharacter = Cast<ACombatCharacter>(InPawn);

	Init();
}

void ACombatAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentDeltaTime = DeltaTime;

	if (OwningCombatCharacter)
	{
		// set the camera to always be placed by the head
		OwningCombatCharacter->FollowCamera->SetWorldLocation(OwningCombatCharacter->GetMesh()->GetBoneLocation("j_head", EBoneSpaces::WorldSpace));

		UpdateCharacterMovement();

		FollowCommanderOrder();

		if (PerceptionComp != nullptr)
		{
			SetVisionAngle();
			ShootAtEnemy();
		}
	}
}


void ACombatAIController::UpdateCharacterMovement()
{
	if (OwningCombatCharacter->IsInHelicopter())
	{
		OwningCombatCharacter->GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;
	}
	else
	{
		OwningCombatCharacter->GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Walking;
	}
}

UAISenseConfig* ACombatAIController::GetPerceptionSenseConfig(TSubclassOf<UAISense> SenseClass)
{
	UAISenseConfig* result = nullptr;

	FAISenseID Id = UAISense::GetSenseID(SenseClass);
	if (!Id.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GetPerceptionSenseConfig: Wrong Sense ID"));
	}
	else
	{
		result = PerceptionComp->GetSenseConfig(Id);
	}

	return result;
}


void ACombatAIController::SetVisionAngle()
{
	UAISenseConfig* Config = GetPerceptionSenseConfig(UAISense_Sight::StaticClass());
	if (Config == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("SetSightRange: Config == nullptr"));
	}
	else
	{
		UAISenseConfig_Sight* ConfigSight = Cast<UAISenseConfig_Sight>(Config);

		// Set Vision angle based whether character is in the helicopter
		if (OwningCombatCharacter->IsInHelicopter())
		{
			ConfigSight->PeripheralVisionAngleDegrees = 60.0f;
		}
		else
		{
			ConfigSight->PeripheralVisionAngleDegrees = 180.0f;
		}

		PerceptionComp->RequestStimuliListenerUpdate();
	}

}

AActor* ACombatAIController::FindEnemy()
{
	float TargetSightDistance = 4000.0f;

	TArray<AActor*> ActorsInSight;

	PerceptionComp->GetKnownPerceivedActors(TSubclassOf<UAISense_Sight>(), ActorsInSight);


	for (int index = 0; index < ActorsInSight.Num(); index++)
	{
		AActor* CurrentActor = ActorsInSight[index];

		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(CurrentActor->GetComponentByClass(UHealthComponent::StaticClass()));

		if (CurrentHealth)
		{
			bool IsAlive = CurrentHealth->getCurrentHealth() > 0.0f;
			bool IsEnemy = !UHealthComponent::IsFriendly(OwningCombatCharacter, CurrentActor);


			if (IsEnemy && IsAlive)
			{
				float PawnLocation = OwningCombatCharacter->GetActorLocation().Size();
				float EnemyLocation = CurrentActor->GetActorLocation().Size();

				auto DistanceDiff = PawnLocation - EnemyLocation;

				if (DistanceDiff < TargetSightDistance)
				{
					TargetSightDistance = DistanceDiff;

					OwningCombatCharacter->TargetFound();

					return CurrentActor;
				}
			}
		}
	}

	return nullptr;
}

void ACombatAIController::ShootAtEnemy()
{
	if (OwningCombatCharacter->IsRepellingDown())
	{
		OwningCombatCharacter->EndFire();
		OwningCombatCharacter->EndAim();
		OwningCombatCharacter->IsInCombatMode(false);
		return;
	}

	CurrentWeapon = OwningCombatCharacter->GetCurrentWeapon();

	if (CurrentWeapon)
	{
		// set unlimited ammo
		if (!OwningCombatCharacter->GetCurrentWeapon()->GetHasUnlimitedAmmo())
			OwningCombatCharacter->GetCurrentWeapon()->SetUnlimitedAmmo(true);

		if (OwningCombatCharacter->IsReloading())
		{
			OwningCombatCharacter->EndFire();
			OwningCombatCharacter->EndAim();
		}


		AActor* EnemyActor = FindEnemy();

		if (EnemyActor)
		{
			SetFocus(EnemyActor);

			OwningCombatCharacter->BeginAim();

			if (CurrentWeapon->getCurrentAmmo() <= 0)
			{
				// check if enemy distance is close, if so then pull out pistol
				// otherwise reload

				float PawnLocation = OwningCombatCharacter->GetActorLocation().Size();
				float EnemyLocation = EnemyActor->GetActorLocation().Size();

				float DistanceDiff = UKismetMathLibrary::Abs(PawnLocation - EnemyLocation);

				float randomDistanceLimit = FMath::RandRange(800.0f, 1000.0f);

				if (DistanceDiff < randomDistanceLimit && CurrentWeapon != OwningCombatCharacter->GetSecondaryWeaponObj() && !OwningCombatCharacter->IsInHelicopter())
				{
					OwningCombatCharacter->EndFire();
					OwningCombatCharacter->EndAim();
					OwningCombatCharacter->BeginWeaponSwap();
				}
				else
				{
					OwningCombatCharacter->BeginReload();
				}
			}
			else
			{
				// Shotguns requires bolt action rather than constant firing of weapon
				// check if using shotgun weapon type 
				AShotgun* ShotgunObj = Cast<AShotgun>(OwningCombatCharacter->GetCurrentWeapon());

				if (ShotgunObj)
				{
					if (ShotgunObj->HasLoadedShell())
					{
						StartFiring();
					}
					else
					{
						OwningCombatCharacter->EndFire();
					}
				}
				else
				{
					StartFiring();
				}
			}


		}
		else
		{

			OwningCombatCharacter->EndFire();
			OwningCombatCharacter->EndAim();

			if (CurrentWeapon->getCurrentAmmo() <= 0) // replenish clip if finished completely
			{
				OwningCombatCharacter->BeginReload();
			}
			else if (CurrentWeapon->getCurrentAmmo() < CurrentWeapon->getAmmoPerClip()) // replenish if not on full clip
			{
				//GetWorldTimerManager().SetTimer(THandler_TimeReloadWeapon, this, &ACombatAIController::ReloadWeapon, FMath::RandRange(5.0f, 10.0f), false, 0.0f);
			}
			else
			{
				// switch back to primary
				if (CurrentWeapon == OwningCombatCharacter->GetSecondaryWeaponObj())
				{
					OwningCombatCharacter->BeginWeaponSwap();
				}
			}
		}

	}

}

void ACombatAIController::StartFiring()
{
	if (BulletFireCountDown > 0.0f)
	{
		if (!IsCoolingDown)
		{
			OwningCombatCharacter->BeginFire();
			BulletFireCountDown -= CurrentDeltaTime;
		}
	}
	else
	{
		OwningCombatCharacter->EndFire();
		IsCoolingDown = true;
	}

	if (IsCoolingDown)
	{
		if (BulletFireCountDown <= FiringWaitTime)
		{
			BulletFireCountDown += CurrentDeltaTime;
		}
		else
		{
			IsCoolingDown = false;
			FiringWaitTime = FMath::RandRange(1.0f, 3.0f);
		}
	}
}

void ACombatAIController::MoveToTarget()
{
	if (OwningCombatCharacter->IsInHelicopter()) {
		return;
	}

	FVector OwnerLocation = OwningCombatCharacter->GetActorLocation();

	MoveToLocation(TargetDestination, AcceptanceRadius, StopOnOverlap, UsePathfinding, ProjectDestinationToNavigation, CanStrafe, FilterClass, AllowPartialPaths);

	float CurrentTargetDistance = UKismetMathLibrary::Vector_Distance(OwnerLocation, TargetDestination);

	if (CurrentTargetDistance > DistanceDiffSprint)
	{
		OwningCombatCharacter->BeginSprint();
	}
	else
	{
		OwningCombatCharacter->EndSprint();
	}
}

void ACombatAIController::FindCover()
{
}

void ACombatAIController::FollowCommanderOrder()
{
	if (OwningCombatCharacter->getCommander() == nullptr) {
		return;
	}
	FCommanderRecruit CommanderRecruit = OwningCombatCharacter->getCommander()->GetRecruitInfo(OwningCombatCharacter);

	if (CommanderRecruit.Recruit == OwningCombatCharacter)
	{
		switch (CommanderRecruit.CurrentCommand)
		{
		case  CommanderOrders::Attack:
		case CommanderOrders::Defend:
		case CommanderOrders::Follow:
			CommanderRecruit.TargetLocation = OwningCombatCharacter->getCommander()->GetActorLocation();
			break;
		}

		TargetDestination = CommanderRecruit.TargetLocation;
		MoveToTarget();
	}

}
// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/CombatAIController.h"

#include "Managers/GameModeManager.h"
#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/Shotgun.h"

#include "Kismet/KismetMathLibrary.h"

#include "CustomComponents/HealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/EngineTypes.h"

#include "NavigationSystem.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "Navigation/PathFollowingComponent.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "..\..\Public\Controllers\CombatAIController.h"

#include "DrawDebugHelpers.h"

ACombatAIController::ACombatAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	AcceptanceRadius = 300.0f;
	DistanceDiffSprint = 200.0f;
	CoverRadius = 10000.0f;
	NumberOfCoverTraces = 35.0f;
	TargetSightRadius = 7000.0f;
}

void ACombatAIController::Init()
{
	if (OwningCombatCharacter)
	{
		UpdateCharacterMovement();

		PerceptionComp = Cast<UAIPerceptionComponent>(OwningCombatCharacter->GetComponentByClass(UAIPerceptionComponent::StaticClass()));

		OwningCombatCharacter->GetCharacterMovement()->bUseRVOAvoidance = true;

		if (TargetSightSphere == nullptr)
		{
			TargetSightSphere = NewObject<USphereComponent>(OwningCombatCharacter);
			if (TargetSightSphere)
			{
				TargetSightSphere->RegisterComponent();
				TargetSightSphere->AttachToComponent(OwningCombatCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				TargetSightSphere->SetSphereRadius(TargetSightRadius);
				TargetSightSphere->SetCollisionProfileName(TEXT("OverlapAll"));
			}
		}

	}
}

void ACombatAIController::BeginPlay()
{
	Super::BeginPlay();

	GameModeManager = Cast<AGameModeManager>(GetWorld()->GetAuthGameMode());

	OwningCombatCharacter = Cast<ACombatCharacter>(AController::GetPawn());

	StayCombatAlert = false;

	Init();

	CurrentDeltaTime = 0.0f;
	BulletFireCountDown = 0.0f;
	FiringWaitTime = FMath::RandRange(3.0f, 5.0f);

	// run behavior tree if specified
	if (BTAsset)
	{
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

		UpdatCombatAlert();

		CheckCommanderOrder();

		if (PerceptionComp != nullptr)
		{
			SetVisionAngle();
			ShootAtEnemy();
		}

		//if (EnemyActor != nullptr)
		//{
		//	FindCover(EnemyActor);
		//	HasChosenCover = false;
		//}
		//else
		//{
		//	// pick a random cover point
		//	if (!HasChosenCover)
		//	{
		//		FindCover(OwningCombatCharacter);
		//	}
		//}
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

void ACombatAIController::UpdatCombatAlert()
{
	if (OwningCombatCharacter->IsFiring())
	{
		StayCombatAlert = true;
	}

	if (OwningCombatCharacter->IsReloading() || OwningCombatCharacter->IsSprinting())
	{
		StayCombatAlert = false;
	}

	if (StayCombatAlert)
	{
		OwningCombatCharacter->BeginAim();
	}
	else
	{
		OwningCombatCharacter->EndAim();
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
	AActor* CurrentActor = nullptr;
	TArray<AActor*> ActorsInSight;
	float TargetSightDistance = TargetSightRadius;

	if (TargetSightSphere) {
		TargetSightSphere->GetOverlappingActors(ActorsInSight, ABaseCharacter::StaticClass());
	}
	else if (PerceptionComp)
	{
		PerceptionComp->GetKnownPerceivedActors(TSubclassOf<UAISense_Sight>(), ActorsInSight);
	}
	else {
		return CurrentActor;
	}

	for (int index = 0; index < ActorsInSight.Num(); index++)
	{
		AActor* PotentialEnemy = ActorsInSight[index];

		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(PotentialEnemy->GetComponentByClass(UHealthComponent::StaticClass()));

		if (CurrentHealth && CurrentHealth->IsAlive())
		{
			bool IsEnemy = !UHealthComponent::IsFriendly(OwningCombatCharacter, PotentialEnemy);

			// is target alive & an enemy
			if (IsEnemy)
			{
				FVector OwnerLocation = OwningCombatCharacter->GetActorLocation();
				FVector EnemyLocation = PotentialEnemy->GetActorLocation();


				// check if can see the target
				FHitResult HitTargetResult;

				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(OwningCombatCharacter);
				QueryParams.bTraceComplex = true;

				FCollisionObjectQueryParams ObjectParams;
				ObjectParams.AllObjects;

				bool bTargetHit = GetWorld()->LineTraceSingleByObjectType(
					HitTargetResult,
					OwningCombatCharacter->FollowCamera->GetComponentLocation(),
					EnemyLocation,
					ObjectParams,
					QueryParams);

				if (bTargetHit)
				{
					if (HitTargetResult.GetActor() == PotentialEnemy)
					{
						// get closest enemy
						float DistanceDiff = FVector::Dist(OwnerLocation, EnemyLocation);

						if (DistanceDiff < TargetSightDistance)
						{
							TargetSightDistance = DistanceDiff;
							CurrentActor = PotentialEnemy;

							OwningCombatCharacter->TargetFound();
						}
					}

				}
			}
		}
	}
	return CurrentActor;
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

		EnemyActor = FindEnemy();

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

				float randomDistanceLimit = FMath::RandRange(0.0f, 200.0f);

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
			ClearFocus(EAIFocusPriority::Gameplay);
			OwningCombatCharacter->EndFire();

			if (CurrentWeapon->getCurrentAmmo() <= 0) // reload clip if finished completely
			{
				OwningCombatCharacter->BeginReload();
			}
			else if (CurrentWeapon->getCurrentAmmo() < CurrentWeapon->getAmmoPerClip()) // reload if not on full clip
			{
				OwningCombatCharacter->BeginReload();
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

EPathFollowingRequestResult::Type ACombatAIController::MoveToTarget(float AcceptRadius)
{
	if (OwningCombatCharacter->IsInHelicopter())
	{
		return EPathFollowingRequestResult::Failed;
	}

	FVector OwnerLocation = OwningCombatCharacter->GetActorLocation();

	EPathFollowingRequestResult::Type Movment = MoveToLocation(TargetDestination, AcceptRadius, StopOnOverlap, UsePathfinding, ProjectDestinationToNavigation, CanStrafe, FilterClass, AllowPartialPaths);

	float CurrentTargetDistance = UKismetMathLibrary::Vector_Distance(OwnerLocation, TargetDestination);

	if (CurrentTargetDistance > (AcceptanceRadius * 2.5f))
	{
		OwningCombatCharacter->BeginSprint();
	}
	else
	{
		OwningCombatCharacter->EndSprint();
	}

	return Movment;
}

void ACombatAIController::FindCover(AActor* TargetActor)
{
	if (TargetActor == nullptr)
	{
		return;
	}

	CoverLocationPoints.Empty();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;

	float factor = 360.0f / NumberOfCoverTraces;

	// 360 degrees line trace around character
	for (int i = 0; i < NumberOfCoverTraces; i++)
	{

		FNavLocation NavLocation;
		bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(TargetActor->GetActorLocation(), NavLocation);

		if (bOnNavMesh)
		{
			const FQuat CharacterTopRotation = UKismetMathLibrary::MakeRotator(0.0f, 0.0f, i * factor).Quaternion();
			FVector Foward = UKismetMathLibrary::Quat_RotateVector(CharacterTopRotation, FVector(1.0f, 0.0f, 0.0f));

			FVector Start = (Foward * CoverRadius) + NavLocation.Location;

			FHitResult HitResult;
			bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, NavLocation.Location, ECC_Visibility);

			// get all hit results which hit an obstacle
			if (bHit)
			{
				FVector LocationPoint = HitResult.ImpactPoint + (HitResult.ImpactNormal * 50.0f) + FVector(0.0f, 0.0f, 100.0f);

				FWorldCoverPoint CoverLocation = FWorldCoverPoint();
				CoverLocation.Location = LocationPoint;
				CoverLocation.Owner = OwningCombatCharacter;

				if (!GameModeManager->IsCoverPointTaken(CoverLocation))
				{
					if (TargetActor == OwningCombatCharacter)
					{
						CoverLocationPoints.Add(LocationPoint);
					}
					else
					{
						bool CanSeeTarget = false;
						float directionValue = FVector::DotProduct(LocationPoint, TargetActor->GetActorLocation());

						float Offset = 50.0f;
						FHitResult HitTargetResult2, HitTargetResult3;
						bool bTargetHit2 = GetWorld()->LineTraceSingleByObjectType(HitTargetResult2, LocationPoint + FVector(0.0f, Offset, 0.0f), TargetActor->GetActorLocation(), ObjectParams, QueryParams);
						bool bTargetHit3 = GetWorld()->LineTraceSingleByObjectType(HitTargetResult3, LocationPoint + FVector(0.0f, 0.0f, Offset), TargetActor->GetActorLocation(), ObjectParams, QueryParams);

						if (bTargetHit2)
						{
							if (Cast<ACombatCharacter>(HitTargetResult2.GetActor()))
							{
								CanSeeTarget = true;
							}
						}

						if (bTargetHit3)
						{
							if (Cast<ACombatCharacter>(HitTargetResult3.GetActor()))
							{
								CanSeeTarget = true;
							}
						}

						if (CanSeeTarget)
						{
							CoverLocationPoints.Add(LocationPoint);
						}
					}
				}
			}
		}
	}

	if (CoverLocationPoints.Num() > 0)
	{
		TargetDestination = GetClosestCoverPoint(TargetActor);
		HasChosenCover = true;
	}

	//TakeCover();
}

FVector ACombatAIController::GetClosestCoverPoint(AActor* TargetActor)
{
	FVector ClosestPoint;
	float minDist = CoverRadius;

	for (int i = 0; i < CoverLocationPoints.Num(); i++)
	{
		FVector Point = CoverLocationPoints[i];

		float Distance = FVector::Dist(OwningCombatCharacter->GetActorLocation(), Point);

		if (Distance < minDist)
		{
			ClosestPoint = Point;
			minDist = Distance;
		}
	}

	// update current cover point, 
	FWorldCoverPoint CoverLocation = FWorldCoverPoint();
	CoverLocation.Owner = OwningCombatCharacter;

	// remove previous cover point from the list
	CoverLocation.Location = ChosenCoverPoint;
	GameModeManager->RemoveCoverPoint(CoverLocation);

	// add the new cover point
	CoverLocation.Location = ClosestPoint;
	GameModeManager->AddCoverPoint(CoverLocation);

	ChosenCoverPoint = ClosestPoint;

	return ClosestPoint;
}

void ACombatAIController::TakeCover()
{
	if (CoverLocationPoints.Num() <= 0)
	{
		// if (!OwningCombatCharacter->GetCharacterMovement()->IsCrouching())
		// 	OwningCombatCharacter->BeginCrouch();
	}
	else
	{
		// if (OwningCombatCharacter->GetCharacterMovement()->IsCrouching())
		// 	OwningCombatCharacter->BeginCrouch();

		EPathFollowingRequestResult::Type Movment = MoveToTarget(0.0f);

		// if (Movment == EPathFollowingRequestResult::AlreadyAtGoal)
		// {
		// 	if (!OwningCombatCharacter->IsTakingCover())
		// 		OwningCombatCharacter->TakeCover();
		// }
		// else
		// {
		// 	if (OwningCombatCharacter->IsTakingCover())
		// 		OwningCombatCharacter->EscapeCover();
		// }
	}
}

void ACombatAIController::CheckCommanderOrder()
{
	Commander = OwningCombatCharacter->getCommander();

	if (Commander == nullptr)
	{
		return;
	}

	FCommanderRecruit CommanderRecruit = Commander->GetRecruitInfo(OwningCombatCharacter);

	if (CommanderRecruit.Recruit != nullptr && CommanderRecruit.Recruit == OwningCombatCharacter)
	{
		StayCombatAlert = true;

		switch (CommanderRecruit.CurrentCommand)
		{
		case CommanderOrders::Attack:
		case CommanderOrders::Defend:
			TargetDestination = CommanderRecruit.TargetLocation;
			break;
		case CommanderOrders::Follow:
			TargetDestination = Commander->GetActorLocation();
			break;
		}

		MoveToTarget(AcceptanceRadius);
	}
}
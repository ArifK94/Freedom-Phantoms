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

#include "NavigationSystem.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"

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

	StayCombatAlert = false;

	Init();

	CurrentDeltaTime = 0.0f;
	BulletFireCountDown = 0.0f;
	FiringWaitTime = FMath::RandRange(3.0f, 5.0f);

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

		UpdatCombatAlert();

		CheckCommanderOrder();

		if (PerceptionComp != nullptr)
		{
			SetVisionAngle();
			ShootAtEnemy();
		}


		//FindHidingSpot();

		if (EnemyActor != nullptr) {
			FindCover(EnemyActor);
		}
		else
		{
			// pick a random cover point
			if (!HasChosenCover) {
				FindCover(OwningCombatCharacter);
			}
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

void ACombatAIController::UpdatCombatAlert()
{
	if (OwningCombatCharacter->IsFiring()) {
		StayCombatAlert = true;
	}

	if (OwningCombatCharacter->IsReloading() || OwningCombatCharacter->IsSprinting()) {
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

EPathFollowingRequestResult::Type ACombatAIController::MoveToTarget(float AcceptRadius)
{
	if (OwningCombatCharacter->IsInHelicopter()) {
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

void ACombatAIController::FindHidingSpot()
{
	FEnvQueryRequest HidingSpotQueryRequest = FEnvQueryRequest(FindHidingSpotEQS, GetPawn());
	HidingSpotQueryRequest.Execute(EEnvQueryRunMode::SingleResult, this, &ACombatAIController::MoveToQueryResult);
}

void ACombatAIController::MoveToQueryResult(TSharedPtr<FEnvQueryResult> result)
{
	if (result->IsSuccsessful()) {
		TargetDestination = result->GetItemAsLocation(0);
		//MoveToLocation(result->GetItemAsLocation(0));

		EPathFollowingRequestResult::Type Movment = MoveToTarget(0.0f);

		if (Movment == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			//	OwningCombatCharacter->IsTakingCover(true);
		}

	}
}

void ACombatAIController::FindCover(AActor* TargetActor)
{
	if (TargetActor == nullptr) {
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
				FVector LocationSet = HitResult.ImpactPoint + (HitResult.ImpactNormal * 50.0f) + FVector(0.0f, 0.0f, 100.0f);

				if (TargetActor == OwningCombatCharacter)
				{
					CoverLocationPoints.Add(LocationSet);
				}
				else
				{
					bool CanSeeTarget = false;
					float directionValue = FVector::DotProduct(LocationSet, TargetActor->GetActorLocation());

					float Offset = 50.0f;
					FHitResult HitTargetResult1, HitTargetResult2, HitTargetResult3;
					bool bTargetHit1 = GetWorld()->LineTraceSingleByObjectType(HitTargetResult1, LocationSet + FVector(Offset, 0.0f, 0.0f), TargetActor->GetActorLocation(), ObjectParams, QueryParams);
					bool bTargetHit2 = GetWorld()->LineTraceSingleByObjectType(HitTargetResult2, LocationSet + FVector(0.0f, Offset, 0.0f), TargetActor->GetActorLocation(), ObjectParams, QueryParams);
					bool bTargetHit3 = GetWorld()->LineTraceSingleByObjectType(HitTargetResult3, LocationSet + FVector(0.0f, 0.0f, Offset), TargetActor->GetActorLocation(), ObjectParams, QueryParams);


					//if (bTargetHit1)
					//{
					//	if (Cast<ACombatCharacter>(HitTargetResult1.GetActor())) {
					//		CanSeeTarget = true;
					//	}
					//}

					if (bTargetHit2)
					{
						if (Cast<ACombatCharacter>(HitTargetResult2.GetActor())) {
							CanSeeTarget = true;
						}
					}


					if (bTargetHit3)
					{
						if (Cast<ACombatCharacter>(HitTargetResult3.GetActor())) {
							CanSeeTarget = true;
						}
					}

					if (CanSeeTarget)
					{
						CoverLocationPoints.Add(LocationSet);
					}
				}
			}
		}
	}


	if (CoverLocationPoints.Num() > 0) {
		TargetDestination = GetClosestCoverPoint(TargetActor);
		HasChosenCover = true;
	}

	TakeCover();
}

FVector ACombatAIController::GetClosestCoverPoint(AActor* TargetActor)
{
	FVector ClosestPoint;
	float minDist = CoverRadius;

	for (int i = 0; i < CoverLocationPoints.Num(); i++)
	{
		FVector Point = CoverLocationPoints[i];

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		WeaponObj = GetWorld()->SpawnActor<AActor>(WeaponClass, Point, FRotator::ZeroRotator, SpawnParams);
		WeaponObj->SetLifeSpan(1.0f);

		float Distance = FVector::Dist(OwningCombatCharacter->GetActorLocation(), Point);

		if (Distance < minDist)
		{
			ClosestPoint = Point;
			minDist = Distance;
		}
	}
	return ClosestPoint;
}

void ACombatAIController::TakeCover()
{
	if (CoverLocationPoints.Num() <= 0)
	{
		//if (!OwningCombatCharacter->GetCharacterMovement()->IsCrouching())
		//	OwningCombatCharacter->BeginCrouch();
	}
	else
	{
		//if (OwningCombatCharacter->GetCharacterMovement()->IsCrouching())
		//	OwningCombatCharacter->BeginCrouch();

		EPathFollowingRequestResult::Type Movment = MoveToTarget(0.0f);

		if (Movment == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			if (!OwningCombatCharacter->IsTakingCover())
				OwningCombatCharacter->IsTakingCover(true);
		}
	}
}

void ACombatAIController::CheckCommanderOrder()
{
	Commander = OwningCombatCharacter->getCommander();

	if (Commander == nullptr) {
		return;
	}

	FCommanderRecruit CommanderRecruit = Commander->GetRecruitInfo(OwningCombatCharacter);

	if (CommanderRecruit.Recruit != nullptr && CommanderRecruit.Recruit == OwningCombatCharacter)
	{
		StayCombatAlert = true;

		switch (CommanderRecruit.CurrentCommand)
		{
		case  CommanderOrders::Attack:
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
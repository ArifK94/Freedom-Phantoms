#include "Controllers/CombatAIController.h"

#include "Managers/GameModeManager.h"
#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/Shotgun.h"
#include "Props/Stronghold.h"
#include "CustomComponents/CoverPointComponent.h"

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

	MovementDebugSphereRadius = 10.0f;
	MovementDebugLifetTime = 1.0f;

	TimeBetweenShotsMin = 2.0f;
	TimeBetweenShotsMax = 3.0f;

	ResetMovementCountdown = 5.0f;
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
	CurrentResetMovementCountdown = ResetMovementCountdown;

	GameModeManager = Cast<AGameModeManager>(GetWorld()->GetAuthGameMode());

	OwningCombatCharacter = Cast<ACombatCharacter>(AController::GetPawn());

	StayCombatAlert = false;

	Init();

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

	CanFindCover = true;

	if (OwningCombatCharacter)
	{
		GetWorldTimerManager().SetTimer(THandler_FollowCamera, this, &ACombatAIController::UpdateFollowCamera, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_BeginFire, this, &ACombatAIController::ShootAtEnemy, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_EndFire, this, &ACombatAIController::EndFiring, FMath::RandRange(TimeBetweenShotsMin, TimeBetweenShotsMax), true);
		GetWorldTimerManager().SetTimer(THandler_FindEnemy, this, &ACombatAIController::FindEnemy, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_CommanderOrders, this, &ACombatAIController::CheckCommanderOrder, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_Sprint, this, &ACombatAIController::UpdateSprint, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_CombatAlert, this, &ACombatAIController::UpdatCombatAlert, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_FindCover, this, &ACombatAIController::FindCover, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_ResetMovement, this, &ACombatAIController::ResetLocation, 2.0f, true);
	}
}

void ACombatAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OwningCombatCharacter)
	{
		UpdateCharacterMovement();

		//if (PerceptionComp != nullptr)
		//{
		//	SetVisionAngle();
		//}
	}
}

void ACombatAIController::UpdateFollowCamera()
{
	// set the camera to always be placed by the head
	OwningCombatCharacter->FollowCamera->SetWorldLocation(OwningCombatCharacter->GetMesh()->GetBoneLocation(OwningCombatCharacter->GetHeadSocket(), EBoneSpaces::WorldSpace));
}

EPathFollowingRequestResult::Type ACombatAIController::MoveToTarget(float AcceptRadius)
{
	if (OwningCombatCharacter->IsInHelicopter())
	{
		return EPathFollowingRequestResult::Failed;
	}

	EPathFollowingRequestResult::Type Movment = MoveToLocation(TargetDestination, AcceptRadius, StopOnOverlap, UsePathfinding, ProjectDestinationToNavigation, CanStrafe, FilterClass, AllowPartialPaths);

	return Movment;
}


void ACombatAIController::UpdateSprint()
{
	FVector OwnerLocation = OwningCombatCharacter->GetActorLocation();

	float CurrentTargetDistance = UKismetMathLibrary::Vector_Distance(OwnerLocation, TargetDestination);

	if (CurrentTargetDistance > AcceptanceRadius)
	{
		OwningCombatCharacter->BeginSprint();
	}
	else
	{
		OwningCombatCharacter->EndSprint();
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

	if (OwningCombatCharacter->IsReloading() || (OwningCombatCharacter->IsSprinting() && EnemyActor == nullptr))
	{
		StayCombatAlert = false;
	}

	if (StayCombatAlert)
	{
		OwningCombatCharacter->BeginAim();
	}
	else
	{
		if (EnemyActor == nullptr)
		{
			OwningCombatCharacter->EndAim();
		}
	}

	if (OwningCombatCharacter->IsAtCoverCorner())
	{
		if (!THandler_BeginPeakCover.IsValid())
		{
			GetWorldTimerManager().SetTimer(THandler_BeginPeakCover, this, &ACombatAIController::BeginCoverPeak, 2.0f, false);
		}


	}
}

void ACombatAIController::BeginCoverPeak()
{
	if (OwningCombatCharacter->IsFacingCoverRHS())
	{
		OwningCombatCharacter->SetRightInputValue(1.0f);
	}
	else
	{
		OwningCombatCharacter->SetRightInputValue(-1.0f);
	}

	GetWorldTimerManager().SetTimer(THandler_EndPeakCover, this, &ACombatAIController::BeginCoverPeak, FMath::RandRange(2.0f, 4.0f), false);
	GetWorldTimerManager().ClearTimer(THandler_BeginPeakCover);
}

void ACombatAIController::EndCoverPeak()
{
	GetWorldTimerManager().ClearTimer(THandler_EndPeakCover);

	OwningCombatCharacter->SetRightInputValue(.0f);
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

void ACombatAIController::FindEnemy()
{
	AActor* CurrentActor = nullptr;
	TArray<AActor*> ActorsInSight;
	float TargetSightDistance = TargetSightRadius;

	if (TargetSightSphere) {
		TargetSightSphere->GetOverlappingActors(ActorsInSight, ABaseCharacter::StaticClass());
	}
	//else if (PerceptionComp)
	//{
	//	PerceptionComp->GetKnownPerceivedActors(TSubclassOf<UAISense_Sight>(), ActorsInSight);
	//}
	//else {
	//	return CurrentActor;
	//}

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

	EnemyActor = CurrentActor;
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

	if (CurrentWeapon == nullptr) {
		return;
	}

	// set unlimited ammo
	if (!OwningCombatCharacter->GetCurrentWeapon()->GetHasUnlimitedAmmo())
		OwningCombatCharacter->GetCurrentWeapon()->SetUnlimitedAmmo(true);

	if (OwningCombatCharacter->IsReloading())
	{
		OwningCombatCharacter->EndFire();
		OwningCombatCharacter->EndAim();
	}

	if (EnemyActor)
	{
		SetFocus(EnemyActor);

		OwningCombatCharacter->BeginAim();

		if (CurrentWeapon->getCurrentAmmo() <= 0)
		{
			// check if enemy distance is close, if so then pull out pistol
			// otherwise reload
			float DistanceDiff = FVector::Dist(OwningCombatCharacter->GetActorLocation(), EnemyActor->GetActorLocation());

			float randomDistanceLimit = FMath::RandRange(0.0f, 20.0f);

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
			ShotgunObj = Cast<AShotgun>(OwningCombatCharacter->GetCurrentWeapon());

			if (ShotgunObj)
			{
				if (ShotgunObj->HasLoadedShell())
				{
					OwningCombatCharacter->BeginFire();
				}
				else
				{
					OwningCombatCharacter->EndFire();
				}
			}
			else
			{
				OwningCombatCharacter->BeginFire();
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

// End fire for non pump-action weapons like shotguns
void ACombatAIController::EndFiring()
{
	if (OwningCombatCharacter == nullptr) {
		return;
	}

	if (ShotgunObj) {
		return;
	}

	if (OwningCombatCharacter->IsFiring())
	{
		OwningCombatCharacter->EndFire();
	}
}

void ACombatAIController::FindCover()
{
	if (!CanFindCover) {
		return;
	}


	if (EnemyActor)
	{
		if (!HasChosenCover)
		{
			//			GenerateCoverPoints(EnemyActor);
		}
	}
	else
	{

		if (!HasChosenCover)
		{
			if (CurrentStronghold)
			{
				ChosenCoverPointComponent = CurrentStronghold->GetCoverPoint(OwningCombatCharacter);

				if (ChosenCoverPointComponent)
				{
					TargetDestination = ChosenCoverPointComponent->GetComponentLocation();
					CoverLocationPoints.Add(ChosenCoverPointComponent->GetComponentLocation());
				}
				else
				{
					//	GenerateCoverPoints(OwningCombatCharacter);
				}

				HasChosenCover = true;
			}
			else
			{	// pick a random cover point
				//GenerateCoverPoints(OwningCombatCharacter);
				HasChosenCover = true;
			}
		}
	}

	// check if current cover has been taken,
	// if so, then find another cover point
	FWorldCoverPoint CoverLocation = FWorldCoverPoint();
	CoverLocation.Location = TargetDestination;
	CoverLocation.Owner = OwningCombatCharacter;

	if (GameModeManager->IsCoverPointTaken(CoverLocation))
	{
		HasChosenCover = false;
	}

	CurrentMovement = MoveToTarget(0.0f);

	TakeCover();

}

void ACombatAIController::GenerateCoverPoints(AActor* TargetActor)
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


			DrawDebugLine(GetWorld(), Start, NavLocation.Location, FColor::Emerald, true, 5.0f, 0, 1.0f);

			// get all hit results which hit an obstacle
			if (bHit)
			{
				//FVector LocationPoint = HitResult.ImpactPoint + (HitResult.ImpactNormal * 50.0f) + FVector(0.0f, 0.0f, 100.0f);
				FVector LocationPoint = HitResult.ImpactPoint;

				DrawDebugLine(GetWorld(), Start, LocationPoint, FColor::Magenta, true, 5.0f, 0, 1.0f);

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
		DrawDebugSphere(GetWorld(), TargetDestination, MovementDebugSphereRadius, 20, FColor::Purple, false, MovementDebugLifetTime, 0, 2);

		if (OwningCombatCharacter->GetCharacterMovement()->IsCrouching())
			OwningCombatCharacter->GetCharacterMovement()->UnCrouch();
	}
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
		if (!OwningCombatCharacter->GetCharacterMovement()->IsCrouching())
			OwningCombatCharacter->BeginCrouch();
		return;
	}

	if (OwningCombatCharacter->GetCharacterMovement()->IsCrouching())
		OwningCombatCharacter->GetCharacterMovement()->UnCrouch();


	switch (CurrentMovement)
	{
	case EPathFollowingRequestResult::Failed:
		HasChosenCover = false;
		break;
	case EPathFollowingRequestResult::AlreadyAtGoal:

		if (ChosenCoverPointComponent)
		{
			FLatentActionInfo LatentInfo;
			LatentInfo.CallbackTarget = this;

			UKismetSystemLibrary::MoveComponentTo(
				OwningCombatCharacter->GetCapsuleComponent(),
				ChosenCoverPointComponent->GetComponentLocation(),
				UKismetMathLibrary::MakeRotFromXZ(FVector(0.0f, ChosenCoverPointComponent->GetRelativeLocation().Z, 0.0f), OwningCombatCharacter->GetCapsuleComponent()->GetUpVector()),
				false,
				false,
				.2f,
				false,
				EMoveComponentAction::Type::Move,
				LatentInfo
			);


			if (ChosenCoverPointComponent->IsCrouchPreferred())
			{
				if (!OwningCombatCharacter->GetCharacterMovement()->IsCrouching()) {
					OwningCombatCharacter->BeginCrouch();
				}
			}

			if (ChosenCoverPointComponent->IsACornerLeft() && ChosenCoverPointComponent->IsACornerRight())
			{
				// choose a random corner direction
				int RandomNumber = FMath::RandRange(0, 1);

				if (RandomNumber == 0)
				{
					OwningCombatCharacter->IsTakingCover(true);
					OwningCombatCharacter->IsAtCoverCorner(true);
					OwningCombatCharacter->IsFacingCoverRHS(false);
				}
				else
				{
					OwningCombatCharacter->IsTakingCover(true);
					OwningCombatCharacter->IsAtCoverCorner(true);
					OwningCombatCharacter->IsFacingCoverRHS(true);
				}
			}

			if (ChosenCoverPointComponent->IsACornerLeft())
			{
				OwningCombatCharacter->IsTakingCover(true);
				OwningCombatCharacter->IsAtCoverCorner(true);
				OwningCombatCharacter->IsFacingCoverRHS(false);
			}
			else if (ChosenCoverPointComponent->IsACornerRight())
			{
				OwningCombatCharacter->IsTakingCover(true);
				OwningCombatCharacter->IsAtCoverCorner(true);
				OwningCombatCharacter->IsFacingCoverRHS(true);
			}
		}
		GetWorldTimerManager().ClearTimer(THandler_FindCover);


		break;
	case EPathFollowingRequestResult::RequestSuccessful:
		HasChosenCover = true;
		break;
	default:
		break;
	}

}

void ACombatAIController::CheckCommanderOrder()
{
	Commander = OwningCombatCharacter->getCommander();

	if (Commander == nullptr)
	{
		return;
	}

	UCommanderRecruit* CommanderRecruit = Commander->GetRecruitInfo(OwningCombatCharacter);

	if (CommanderRecruit != nullptr && CommanderRecruit->Recruit != nullptr && CommanderRecruit->Recruit == OwningCombatCharacter)
	{
		StayCombatAlert = true;

		float TargetRadius = AcceptanceRadius;

		switch (CommanderRecruit->CurrentCommand)
		{
		case CommanderOrders::Attack:
			TargetDestination = CommanderRecruit->TargetLocation;
			CanFindCover = true;
			break;
		case CommanderOrders::Defend:
			TargetDestination = CommanderRecruit->TargetLocation;
			TargetRadius = 0.0f;
			CanFindCover = true;
			break;
		case CommanderOrders::Follow:
			TargetDestination = Commander->GetActorLocation();
			CanFindCover = false;

			// Crouch if the commander is crouched
			if (Commander->GetCharacterMovement()->IsCrouching())
			{
				if (!OwningCombatCharacter->GetCharacterMovement()->IsCrouching())
					OwningCombatCharacter->Crouch();
			}
			else
			{
				if (OwningCombatCharacter->GetCharacterMovement()->IsCrouching())
					OwningCombatCharacter->UnCrouch();
			}

			break;
		}


		CurrentMovement = MoveToTarget(TargetRadius);
	}
}

void ACombatAIController::ResetLocation()
{
	if (CurrentMovement == EPathFollowingRequestResult::AlreadyAtGoal) {
		return;
	}

	// Using character speed to check if stuck or not
	if (OwningCombatCharacter->GetCharacterSpeed() <= .2f)
	{
		// if reset is more than 0 then perform countdown
		if (CurrentResetMovementCountdown > 0.0f)
		{
			CurrentResetMovementCountdown--;
		}
		else
		{
			// reset movement
			// find closest location point on navmesh to current character location
			FNavLocation NavLocation;
			UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
			bool navResult = NavigationArea->ProjectPointToNavigation(OwningCombatCharacter->GetActorLocation(), NavLocation);

			if (navResult)
			{
				OwningCombatCharacter->SetActorLocation(NavLocation.Location);

				// reset countdown timer
				CurrentResetMovementCountdown = ResetMovementCountdown;

				DrawDebugSphere(GetWorld(), NavLocation.Location, 10.0f, 20, FColor::Purple, false, 100.0f, 0, 2);

			}
		}
	}
	else
	{
		CurrentResetMovementCountdown = ResetMovementCountdown;
	}
}
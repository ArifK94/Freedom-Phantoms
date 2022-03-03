#include "Controllers/CombatAIController.h"

#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/PumpActionWeapon.h"
#include "Weapons/MountedGun.h"
#include "Props/Stronghold.h"
#include "CustomComponents/AIMovementComponent.h"
#include "CustomComponents/PatrolFollowerComponent.h"
#include "CustomComponents/CoverFinderComponent.h"
#include "CustomComponents/CoverPointComponent.h"
#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/MountedGunFinderComponent.h"
#include "CustomComponents/HealthComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"
#include "NavigationSystem.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "Navigation/PathFollowingComponent.h"
#include "..\..\Public\Controllers\CombatAIController.h"
#include "GameFramework/CharacterMovementComponent.h"


void ACombatAIController::OnMovementDestinationSet(AIBehaviourState BehaviourState)
{
	SetBehaviourState(BehaviourState);
}

void ACombatAIController::OnMovementDestinationReached(FVector Destination)
{
	switch (CurrentBehaviourState)
	{
	case AIBehaviourState::Patrol:
		MoveToNextPatrolPoint();
		return;
	case AIBehaviourState::MovingToLastSeenEnemy:
		GetWorldTimerManager().SetTimer(THandler_LastSeenEnemy, this, &ACombatAIController::UpdateLastSeen, 1.0f, true, FMath::RandRange(2.f, 5.f));
		return;
	}

	if (!OwningCombatCharacter->GetMountedGun())
	{
		GetWorldTimerManager().SetTimer(THandler_MoveToNearbyDestination, this, &ACombatAIController::MoveToRandomPoint, .5f, true);
	}

	GetWorldTimerManager().SetTimer(THandler_MountedGun, this, &ACombatAIController::FindMountedGun, 1.0f, true);

}

void ACombatAIController::OnOrderReceived(UCommanderRecruit* RecruitInfo)
{
	// ensure the owning character received the order
	if (RecruitInfo->Recruit != OwningCombatCharacter) {
		return;
	}

	if (OwningCombatCharacter->IsTakingCover())
	{
		OwningCombatCharacter->StopCover();
	}

	GetWorldTimerManager().ClearTimer(THandler_MountedGun);
	OwningCombatCharacter->DropMountedGun();

	float TargetRadius = AcceptanceRadius;

	CurrentCommand = RecruitInfo->CurrentCommand;
	TargetDestination = RecruitInfo->TargetLocation;

	OwningCombatCharacter->GetCharacterMovement()->bUseRVOAvoidance = true;

	// Defending point should be right where it was ordered to go to
	if (CurrentCommand == CommanderOrders::Defend)
	{
		TargetRadius = 0.0f;
		OwningCombatCharacter->GetCharacterMovement()->bUseRVOAvoidance = false;
	}

	CanFindCover = true;
	HasChosenNearTargetDest = false;


	StayCombatAlert = false;
	UpdatCombatAlert();

	AIMovementComponent->MoveToDestination(TargetDestination, TargetRadius, AIBehaviourState::PriorityOrdersCommander);
}

ACombatAIController::ACombatAIController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	AcceptanceRadius = 300.0f;

	TimeBetweenShotsMin = 2.0f;
	TimeBetweenShotsMax = 3.0f;

	DestinationRadius = 300.0f;

	MoveToLastSeenEnemy = true;

	AIMovementComponent = CreateDefaultSubobject<UAIMovementComponent>(TEXT("AIMovementComponent"));
	CoverFinderComponent = CreateDefaultSubobject<UCoverFinderComponent>(TEXT("CoverFinderComponent"));
}

void ACombatAIController::Init()
{
	if (OwningCombatCharacter == nullptr) {
		return;
	}

	OwningCombatCharacter->GetCharacterMovement()->bUseRVOAvoidance = true;
	OwningCombatCharacter->SetUseAimCameraSpring(false);

	// Remove sprint by default as the custom move to location will toggle sprint based on distance of the destination
	if (OwningCombatCharacter->GetIsSprintDefault()) {
		OwningCombatCharacter->ToggleSprint();
	}

	// Spawned actors do not create the components made in the constructor, so create the components again at runtime
	if (!AIMovementComponent)
	{
		AIMovementComponent = NewObject<UAIMovementComponent>(this);

		if (AIMovementComponent)
		{
			AIMovementComponent->RegisterComponent();
		}
	}


	// We want to add patrol path to existing characters on the level rather than spawning AI characters then assigning them
	auto ActorComponent = OwningCombatCharacter->GetComponentByClass(UPatrolFollowerComponent::StaticClass());

	if (ActorComponent) {
		PatrolFollowerComponent = Cast<UPatrolFollowerComponent>(ActorComponent);
	}

	if (!PatrolFollowerComponent)
	{
		PatrolFollowerComponent = NewObject<UPatrolFollowerComponent>(OwningCombatCharacter);

		if (PatrolFollowerComponent)
		{
			PatrolFollowerComponent->RegisterComponent();
		}
	}

	if (!CoverFinderComponent)
	{
		CoverFinderComponent = NewObject<UCoverFinderComponent>(this);

		if (CoverFinderComponent)
		{
			CoverFinderComponent->RegisterComponent();
		}
	}

	if (!TargetFinderComponent)
	{
		TargetFinderComponent = NewObject<UTargetFinderComponent>(OwningCombatCharacter);

		if (TargetFinderComponent)
		{
			TargetFinderComponent->RegisterComponent();
			TargetFinderComponent->SetFindTargetPerFrame(true);
		}
	}

	if (!MountedGunFinderComponent)
	{
		MountedGunFinderComponent = NewObject<UMountedGunFinderComponent>(OwningCombatCharacter);

		if (MountedGunFinderComponent)
		{
			MountedGunFinderComponent->RegisterComponent();
		}
	}

	HasAssignedOrderEvent = false;

	// if assigned an MG at the beginning
	if (OwningCombatCharacter->GetMountedGun())
	{
		TargetDestination = OwningCombatCharacter->GetMountedGun()->GetActorLocation();
	}
}

EPathFollowingRequestResult::Type ACombatAIController::MoveToTarget(float AcceptRadius, bool WalkNearTarget)
{
	if (TargetDestination.IsZero() || OwningCombatCharacter->GetIsInVehicle() || OwningCombatCharacter->IsUsingMountedWeapon()) {
		return EPathFollowingRequestResult::Failed;
	}

	auto CurrentMovement = MoveToLocation(TargetDestination, AcceptRadius, StopOnOverlap, UsePathfinding, ProjectDestinationToNavigation, CanStrafe, FilterClass, AllowPartialPaths);

	// Walk when close to desination
	if (WalkNearTarget)
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
	else
	{
		if (CurrentMovement != EPathFollowingRequestResult::AlreadyAtGoal)
		{
			OwningCombatCharacter->BeginSprint();
		}
		else
		{
			OwningCombatCharacter->EndSprint();
		}
	}

	return CurrentMovement;
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
		// Attach Follow Camera to head socket
		OwningCombatCharacter->SetFirstPersonView();

		PumpActionWeapon = Cast<APumpActionWeapon>(OwningCombatCharacter->GetPrimaryWeapon());

		GetWorldTimerManager().SetTimer(THandler_PatrolStart, this, &ACombatAIController::StartPatrol, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_MountedGun, this, &ACombatAIController::FindMountedGun, 1.0f, true, 2.0f); // delay finding MG after checking if AI is set for patrol
		GetWorldTimerManager().SetTimer(THandler_CommanderOrders, this, &ACombatAIController::CheckCommanderOrder, 1.0f, true);
		//GetWorldTimerManager().SetTimer(THandler_BeginPeakCover, this, &ACombatAIController::BeginCoverPeak, FMath::RandRange(2.0f, 5.0f), true);


		// Events! DON'T FORGET TO REMOVE EVENTS ON THE UNPOSSESS() method to prevent a crash when switching between possess & unpossess
		TargetFinderComponent->OnTargetSearch.AddDynamic(this, &ACombatAIController::OnTargetSearchUpdate);
		OwningCombatCharacter->OnRappelUpdate.AddDynamic(this, &ACombatAIController::OnRappelUpdated);

		if (AIMovementComponent)
		{
			AIMovementComponent->OnDestinationSet.AddDynamic(this, &ACombatAIController::OnMovementDestinationSet);
			AIMovementComponent->OnDestinationReached.AddDynamic(this, &ACombatAIController::OnMovementDestinationReached);
		}


		UHealthComponent* HealthComp = OwningCombatCharacter->GetHealthComp();

		if (HealthComp)
		{
			HealthComp->OnHealthChanged.AddDynamic(this, &ACombatAIController::OnHealthUpdate);
		}
	}

	// run behavior tree if specified
	if (BTAsset)
	{
		AAIController::RunBehaviorTree(BTAsset);
	}
}

void ACombatAIController::OnUnPossess()
{
	Super::OnUnPossess();

	if (OwningCombatCharacter)
	{
		TargetFinderComponent->OnTargetSearch.RemoveDynamic(this, &ACombatAIController::OnTargetSearchUpdate);
		OwningCombatCharacter->OnRappelUpdate.RemoveDynamic(this, &ACombatAIController::OnRappelUpdated);

		if (AIMovementComponent)
		{
			AIMovementComponent->OnDestinationSet.RemoveDynamic(this, &ACombatAIController::OnMovementDestinationSet);
			AIMovementComponent->OnDestinationReached.RemoveDynamic(this, &ACombatAIController::OnMovementDestinationReached);
		}

		UHealthComponent* HealthComp = OwningCombatCharacter->GetHealthComp();

		if (HealthComp)
		{
			HealthComp->OnHealthChanged.RemoveDynamic(this, &ACombatAIController::OnHealthUpdate);
		}
	}


	ClearTimers();
}

void ACombatAIController::ClearTimers()
{
	TargetFinderComponent->SetFindTargetPerFrame(false);

	GetWorldTimerManager().ClearTimer(THandler_ShootEnemy);
	GetWorldTimerManager().ClearTimer(THandler_EndFire);
	GetWorldTimerManager().ClearTimer(THandler_MountedGun);
	GetWorldTimerManager().ClearTimer(THandler_CommanderOrders);
	GetWorldTimerManager().ClearTimer(THandler_FindCover);
	GetWorldTimerManager().ClearTimer(THandler_PatrolStart);
}


void ACombatAIController::BeginPlay()
{
	Super::BeginPlay();

	DefaultDestinationRadius = DestinationRadius;

	TargetSearchParams = new FTargetSearchParameters();
}

void ACombatAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	m_DelaTime = DeltaTime;

	if (OwningCombatCharacter->GetHealthComp()->IsAlive())
	{
		if (EnemyActor)
		{
			// if using a mounted gun
			if (OwningCombatCharacter->IsUsingMountedWeapon() && OwningCombatCharacter->GetMountedGun())
			{
				MountedGunFinderComponent->FocusTarget(OwningCombatCharacter->GetMountedGun(), EnemyActor->GetActorLocation());
			}
			else
			{
				//SetFocus(EnemyActor);
				SetFocalPoint(TargetSearchParams->TargetLocation);
			}
		}
		else
		{
			if (OwningCombatCharacter->IsUsingMountedWeapon())
			{
				OwningCombatCharacter->GetMountedGun()->SetRotationInput(FRotator::ZeroRotator, 1.5f);
			}
		}
	}
}



void ACombatAIController::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	if (!InHealthParameters.AffectedHealthComponent->IsAlive())
	{
		ClearTimers();

		OwningCombatCharacter->DetachFromControllerPendingDestroy();
	}
}

void ACombatAIController::OnRappelUpdated(ABaseCharacter* BaseCharacter)
{
	// Find a random point when landed after rappellinh so upcoming characters rapelling down do not stand in the same spot
	if (!OwningCombatCharacter->IsRepellingDown())
	{
		FNavLocation NavLocation;
		UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
		bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(OwningCombatCharacter->GetActorLocation(), NavLocation);

		if (bOnNavMesh)
		{
			DestinationRadius = 1000.f;
			TargetDestination = NavLocation.Location;
			GetWorldTimerManager().SetTimer(THandler_MoveToNearbyDestination, this, &ACombatAIController::MoveToRandomPoint, 1.f, true);
		}
		else
		{
			DestinationRadius = DefaultDestinationRadius;
		}
	}
}

void ACombatAIController::OnTargetSearchUpdate(FTargetSearchParameters TargetSearchParameters)
{
	AActor* ChosenTarget = TargetSearchParameters.TargetActor;

	TargetSearchParams->TargetActor = TargetSearchParameters.TargetActor;
	TargetSearchParams->TargetLocation = TargetSearchParameters.TargetLocation;


	if (ChosenTarget) // found an enemy?
	{
		LastSeenEnemyActor = nullptr;

		if (OwningCombatCharacter->IsTakingCover())
		{
			if (TargetFinderComponent->IsTargetBehind(OwningCombatCharacter, ChosenTarget))
			{
				OwningCombatCharacter->StopCover();
			}
		}

		if (OwningCombatCharacter->IsUsingMountedWeapon() && OwningCombatCharacter->GetMountedGun() && !OwningCombatCharacter->GetMountedGun()->GetUseControllerRotationYaw())
		{
			// check if enemy position is NOT within turret's pitch and yaw boundaries or is not behind the MG
			if (!MountedGunFinderComponent->IsInTargetRange(OwningCombatCharacter->GetMountedGun(), OwningCombatCharacter, ChosenTarget) ||
				TargetFinderComponent->IsTargetBehind(OwningCombatCharacter, ChosenTarget))
			{
				// for normal turrets on the ground
				if (OwningCombatCharacter->GetMountedGun()->GetCanExitMG())
				{
					OwningCombatCharacter->DropMountedGun(false);
				}
				else // for aircraft turrets which cannot be exited
				{
					ChosenTarget = nullptr;
				}
			}
		}

		// if this is a new enemy then start end fire again so AI does not have wait to fire again when focusing on new enemy
		if (EnemyActor != ChosenTarget)
		{
			GetWorldTimerManager().ClearTimer(THandler_EndFire);

			if (!THandler_EndFire.IsValid()) {
				GetWorldTimerManager().SetTimer(THandler_EndFire, this, &ACombatAIController::EndFiring, FMath::RandRange(TimeBetweenShotsMin, TimeBetweenShotsMax), true);
			}
		}

		GetWorldTimerManager().ClearTimer(THandler_LastSeenEnemy);
		EnemyActor = ChosenTarget;

		// Being firing at enemy
		if (!THandler_ShootEnemy.IsValid()) {
			GetWorldTimerManager().SetTimer(THandler_ShootEnemy, this, &ACombatAIController::ShootAtEnemy, 1.0f, true);
		}

		// Do find a random point if current command is defend
		// & is set to use the mounted gun
		if (!THandler_MoveToNearbyDestination.IsValid()
			&& CurrentCommand != CommanderOrders::Defend
			&& CurrentBehaviourState != AIBehaviourState::PriorityDestination
			&& !OwningCombatCharacter->GetMountedGun())
		{
			SetBehaviourState(AIBehaviourState::Normal);
			HasChosenNearTargetDest = false;
			TargetDestination = OwningCombatCharacter->GetActorLocation();
			GetWorldTimerManager().SetTimer(THandler_MoveToNearbyDestination, this, &ACombatAIController::MoveToRandomPoint, FMath::RandRange(.5f, 2.f), true);
		}


	}
	else  // not found an enemy
	{
		if (EnemyActor) // if previous enemy still exists, look at the last location it was seen
		{
			LastSeenEnemyActor = EnemyActor;
			LastSeenPosition = LastSeenEnemyActor->GetActorLocation();
			EnemyActor = nullptr;
			GetWorldTimerManager().ClearTimer(THandler_MoveToNearbyDestination);
		}
	}
}

void ACombatAIController::UpdatCombatAlert()
{
	if (StayCombatAlert)
	{
		OwningCombatCharacter->BeginAim();
	}
	else
	{
		OwningCombatCharacter->EndAim();
	}
}

void ACombatAIController::BeginCoverPeak()
{
	if (!OwningCombatCharacter->IsAtCoverCorner()) {
		return;
	}


	if (OwningCombatCharacter->IsFacingCoverRHS())
	{
		OwningCombatCharacter->CoverMovement(1.0f);
	}
	else
	{
		OwningCombatCharacter->CoverMovement(-1.0f);
	}

	if (!THandler_EndPeakCover.IsValid())
	{
		GetWorldTimerManager().SetTimer(THandler_EndPeakCover, this, &ACombatAIController::EndCoverPeak, FMath::RandRange(2.0f, 4.0f), false);
	}
}

void ACombatAIController::EndCoverPeak()
{
	GetWorldTimerManager().ClearTimer(THandler_EndPeakCover);

	OwningCombatCharacter->SetRightInputValue(.0f);
}

void ACombatAIController::UpdateLastSeen()
{
	// look straight ahead with the MG direction
	if (OwningCombatCharacter->IsUsingMountedWeapon() && OwningCombatCharacter->GetMountedGun() && OwningCombatCharacter->GetMountedGun()->GetAdjustBehindMG())
	{
		OwningCombatCharacter->GetCapsuleComponent()->SetWorldRotation(OwningCombatCharacter->GetMountedGun()->GetCharacterStandRot());
	}

	if (OwningCombatCharacter->GetCurrentWeapon()->getCurrentAmmo() <= 0 || OwningCombatCharacter->GetCurrentWeapon()->getCurrentAmmo() < OwningCombatCharacter->GetCurrentWeapon()->getAmmoPerClip()) // reload clip if finished completely or  reload if not on full clip
	{
		OwningCombatCharacter->BeginReload();
	}
	else
	{
		// switch back to primary
		if (OwningCombatCharacter->GetCurrentWeapon() == OwningCombatCharacter->GetSecondaryWeaponObj())
		{
			OwningCombatCharacter->BeginWeaponSwap();
		}
	}

	// Stop aiming if not combat alert
	if (!StayCombatAlert)
	{
		OwningCombatCharacter->EndAim();
	}

	LastSeenEnemyActor = nullptr;

	// Go back to patrolling if not following commander or has no destination set
	if (CurrentBehaviourState != AIBehaviourState::PriorityOrdersCommander &&
		CurrentBehaviourState != AIBehaviourState::PriorityDestination)
	{
		if (!THandler_PatrolStart.IsValid()) {
			GetWorldTimerManager().SetTimer(THandler_PatrolStart, this, &ACombatAIController::StartPatrol, 1.0f, true);
		}
	}

	GetWorldTimerManager().ClearTimer(THandler_LastSeenEnemy);
}

void ACombatAIController::ShootAtEnemy()
{
	if (OwningCombatCharacter->GetCurrentWeapon() == nullptr) {
		return;
	}

	if (OwningCombatCharacter->IsRepellingDown() || OwningCombatCharacter->IsSwappingWeapon() || OwningCombatCharacter->IsReloading())
	{
		/**
		* Remove this if statement when the end reload is triggered as soon as the capacity is reached. Problem arises when this function is returned because the reload was not being ended.
		*/
		if (OwningCombatCharacter->IsReloading())
		{
			if (OwningCombatCharacter->GetCurrentWeapon()->getCurrentAmmo() >= OwningCombatCharacter->GetCurrentWeapon()->getAmmoPerClip())
			{
				OwningCombatCharacter->EndReload();
			}
		}

		OwningCombatCharacter->EndFire();
		OwningCombatCharacter->EndAim();
		OwningCombatCharacter->IsInCombatMode(false);
		return;
	}



	// set unlimited ammo
	if (!OwningCombatCharacter->GetCurrentWeapon()->GetHasUnlimitedAmmo())
	{
		OwningCombatCharacter->GetCurrentWeapon()->SetUnlimitedAmmo(true);
	}

	if (EnemyActor)
	{
		if (!OwningCombatCharacter->IsSprinting())
		{
			OwningCombatCharacter->BeginAim();
		}

		if (OwningCombatCharacter->GetCurrentWeapon()->getCurrentAmmo() <= 0 || OwningCombatCharacter->IsReloading())
		{
			ReloadWeapon();
		}
		else
		{
			// check if enemy distance is close, if so then pull out pistol
			float DistanceDiff = FVector::Dist(OwningCombatCharacter->GetActorLocation(), EnemyActor->GetActorLocation());
			float randomDistanceLimit = FMath::RandRange(500.0f, 1000.0f);
			bool IsTargetClose = DistanceDiff < randomDistanceLimit;

			if (!IsTargetClose && OwningCombatCharacter->GetCurrentWeapon() == OwningCombatCharacter->GetSecondaryWeaponObj() // if target is not close, then switch back to primary
				&& !OwningCombatCharacter->GetIsInVehicle())
			{
				OwningCombatCharacter->BeginWeaponSwap();
			}
			else // Fire the weapon
			{
				// Shotguns require bolt action rather than constant firing of weapon
				// check if using shotgun weapon type

				if (PumpActionWeapon)
				{
					if (PumpActionWeapon->GetHasLoadedShell())
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


	}
	else
	{
		OwningCombatCharacter->EndFire();

		if (OwningCombatCharacter->IsUsingMountedWeapon() && OwningCombatCharacter->GetMountedGun())
		{
			OwningCombatCharacter->EndAim();
		}
		else
		{
			// Focus on the last seen enemy
			if (LastSeenEnemyActor)
			{
				OwningCombatCharacter->BeginAim();

				// Move near nearest enemy last seen
				if (CurrentBehaviourState != AIBehaviourState::PriorityOrdersCommander ||
					CurrentBehaviourState != AIBehaviourState::PriorityDestination ||
					CurrentBehaviourState != AIBehaviourState::MovingToLastSeenEnemy)
				{
					SetBehaviourState(AIBehaviourState::MovingToLastSeenEnemy);


					if (CurrentBehaviourState == AIBehaviourState::MovingToLastSeenEnemy && MoveToLastSeenEnemy)
					{
						if (OwningCombatCharacter->GetMountedGun())
						{
							OwningCombatCharacter->DropMountedGun();
						}

						// Go near the last seen postion on a random radius
						float Radius = FMath::RandRange(500.f, 1000.f);
						TArray<AActor*> IgnoreActors;
						TargetDestination = AIMovementComponent->FindNearbyDestinationPoint(LastSeenPosition, Radius, IgnoreActors);
						AIMovementComponent->MoveToDestination(TargetDestination, Radius, CurrentBehaviourState, false, true);
						SetFocalPoint(TargetSearchParams->TargetLocation);
					}
				}
			}
		}



		GetWorldTimerManager().ClearTimer(THandler_ShootEnemy);
		GetWorldTimerManager().ClearTimer(THandler_EndFire);
	}

}

// End fire for non pump-action weapons like shotguns
void ACombatAIController::EndFiring()
{
	if (PumpActionWeapon) {
		return;
	}

	if (OwningCombatCharacter->IsFiring())
	{
		OwningCombatCharacter->EndFire();
	}
}

void ACombatAIController::ReloadWeapon()
{
	// check if enemy distance is close, if so then pull out pistol
	float DistanceDiff = FVector::Dist(OwningCombatCharacter->GetActorLocation(), EnemyActor->GetActorLocation());
	float randomDistanceLimit = FMath::RandRange(500.0f, 1000.0f);
	bool IsTargetClose = DistanceDiff < randomDistanceLimit;

	if (OwningCombatCharacter->GetCurrentWeapon()->getCurrentAmmo() <= 0 && !PumpActionWeapon) // Reload the weapon
	{
		if (IsTargetClose &&
			OwningCombatCharacter->GetCurrentWeapon() != OwningCombatCharacter->GetSecondaryWeaponObj() && // if using primary weapon & enemy is nearby, then swap to secondary
			!OwningCombatCharacter->GetIsInVehicle())
		{
			OwningCombatCharacter->BeginWeaponSwap();
		}
		else
		{
			OwningCombatCharacter->BeginReload();
		}
	}
	else if (PumpActionWeapon && (OwningCombatCharacter->GetCurrentWeapon()->getCurrentAmmo() <= 0 || OwningCombatCharacter->IsReloading())) // pump action weapons can be fired as soon as the first ammo shell is inserted, so better to add few more bullets first before firing again
	{
		int RandomAmount = rand() % OwningCombatCharacter->GetCurrentWeapon()->getAmmoPerClip();

		if (OwningCombatCharacter->GetCurrentWeapon()->getCurrentAmmo() <= 0 || OwningCombatCharacter->GetCurrentWeapon()->getCurrentAmmo() < RandomAmount)
		{
			OwningCombatCharacter->BeginReload();
		}
		else
		{
			OwningCombatCharacter->EndReload();
		}
	}
	else
	{
		OwningCombatCharacter->EndReload();
	}
}

void ACombatAIController::FindMountedGun()
{
	// if already using an MG
	// if using an aircraft MG for instance, which should not be exited, no need to run this method
	if (OwningCombatCharacter->GetMountedGun() && !OwningCombatCharacter->GetMountedGun()->GetCanExitMG())
	{
		GetWorldTimerManager().ClearTimer(THandler_MountedGun);
		return;
	}

	// If in patrol mode & no enemy in sight then do not search for MG
	if (CurrentBehaviourState == AIBehaviourState::Patrol)
	{
		if (!EnemyActor)
		{
			OwningCombatCharacter->DropMountedGun();
			GetWorldTimerManager().ClearTimer(THandler_MountedGun);
			return;
		}
	}

	// If an MG has been assigned
	if (OwningCombatCharacter->GetMountedGun())
	{
		if (!OwningCombatCharacter->IsReloading() 
			&& !OwningCombatCharacter->IsUsingMountedWeapon() 
			&& !TargetFinderComponent->IsTargetBehind(OwningCombatCharacter, EnemyActor)
			&& MountedGunFinderComponent->IsInTargetRange(OwningCombatCharacter->GetMountedGun(), OwningCombatCharacter, EnemyActor)
			)
		{
			// Don't want AI to teleport to the MG, needs to be close enough to use it
			auto DistanceToMG = FVector::Distance(OwningCombatCharacter->GetActorLocation(), OwningCombatCharacter->GetMountedGun()->GetCharacterStandPos());

			if (DistanceToMG < 100.f)
			{
				OwningCombatCharacter->UseMountedGun();
			}

			return;
		}

		// in case player or another NPC has reached the MG before AI
		if (OwningCombatCharacter->GetMountedGun()->GetOwner() != OwningCombatCharacter ||
			!MountedGunFinderComponent->IsInTargetRange(OwningCombatCharacter->GetMountedGun(), OwningCombatCharacter, EnemyActor)
			)
		{
			OwningCombatCharacter->DropMountedGun();
			CanFindCover = true;
			return;
		}
		else if (TargetFinderComponent->IsTargetBehind(OwningCombatCharacter, EnemyActor))
		{
			// Still posssess MG when enemy is no longer behing AI
			OwningCombatCharacter->DropMountedGun(false);
			CanFindCover = false;
			return;
		}
	}


	bool CanFindMG = true;

	if (Commander && !IsNearCommander()) {
		CanFindMG = false;
	}

	if (MountedGunFinderComponent && !OwningCombatCharacter->GetMountedGun() && CanFindMG)
	{
		auto SelectedMG = MountedGunFinderComponent->FindMG();

		// if found an MG 
		// & enemy is not behind the MG
		if (SelectedMG)
		{
			bool IsMGValid = true;
			// Check if AI has a ollow order, if it's defend or attack then the if statement should be ignored
			// and commander is near the MG,
			if (Commander && CurrentCommand == CommanderOrders::Follow && !IsNearCommander(SelectedMG->GetCharacterStandPos()))
			{
				IsMGValid = false;
			}

			if (EnemyActor && IsMGValid)
			{
				bool IsInRange = MountedGunFinderComponent->IsInTargetRange(SelectedMG, EnemyActor, OwningCombatCharacter);

				if (!IsInRange)
				{
					IsMGValid = false;
				}
				else if (TargetFinderComponent->IsTargetBehind(SelectedMG, EnemyActor))
				{
					IsMGValid = false;
				}
			}

			if (IsMGValid)
			{
				SelectedMG->SetPotentialOwner(OwningCombatCharacter);
				OwningCombatCharacter->SetMountedGun(SelectedMG);
				CanFindCover = false;

				TargetDestination = OwningCombatCharacter->GetMountedGun()->GetCharacterStandPos();
				AIMovementComponent->MoveToDestination(TargetDestination, .0f, AIBehaviourState::Normal, true, false);
			}

		}
	}



}

void ACombatAIController::MoveToCover()
{
	if (!CanFindCover) {
		return;
	}

	if (!HasChosenCover)
	{
		if (EnemyActor)
		{
			auto CoverLocation = CoverFinderComponent->FindCover(EnemyActor->GetActorLocation());

			TargetDestination = CoverLocation;
			HasChosenCover = true;
		}
		else
		{
			if (CurrentStronghold)
			{
				ChosenCoverPointComponent = CurrentStronghold->GetCoverPoint(OwningCombatCharacter);

				if (ChosenCoverPointComponent)
				{
					TargetDestination = ChosenCoverPointComponent->GetComponentLocation();
				}
			}
		}
	}


	//auto CurrentMovement = AIMovementComponent->MoveToDestination(TargetDestination, .0f);
	auto CurrentMovement = MoveToTarget(0.0f);

	switch (CurrentMovement)
	{
	case EPathFollowingRequestResult::Failed:
		HasChosenCover = false;
		break;
	case EPathFollowingRequestResult::AlreadyAtGoal:

		if (!OwningCombatCharacter->GetCharacterMovement()->IsCrouching())
			OwningCombatCharacter->ToggleCrouch();

		GetWorldTimerManager().ClearTimer(THandler_FindCover);

		break;
	case EPathFollowingRequestResult::RequestSuccessful:
		HasChosenCover = true;
		break;
	default:
		break;
	}


	// Choose another cover point if this has already been taken by another
	if (CoverFinderComponent->IsCoverPointTaken(TargetDestination))
	{
		HasChosenCover = false;
	}
}

void ACombatAIController::MoveToRandomPoint()
{
	if (Commander && CurrentCommand == CommanderOrders::Follow) {
		return;
	}


	//auto Movement = AIMovementComponent->MoveToDestination(TargetDestination, .0f);
	auto Movement = MoveToTarget(0.0f);

	//// Find a random point around destination if has arrived at the original target destination
	if (!HasChosenNearTargetDest && Movement == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		TArray<AActor*> IgnoreActors;

		if (Commander) {
			IgnoreActors.Add(Commander);
		}

		auto NearDestination = AIMovementComponent->FindNearbyDestinationPoint(TargetDestination, DestinationRadius, IgnoreActors);

		if (!NearDestination.IsZero())
		{
			TargetDestination = NearDestination; // upate the target destination
			//Movement = MoveToTarget(0.0f); // Move to new destination
			AIMovementComponent->MoveToDestination(NearDestination, .0f, AIBehaviourState::Normal);
			HasChosenNearTargetDest = true;
			GetWorldTimerManager().ClearTimer(THandler_MoveToNearbyDestination);
		}
	}


	// Has chosen a nearby destination
	// & arrvied at the nearby destination
	if (HasChosenNearTargetDest && Movement == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		// Toggle crouch based on random possibility
		int RandonNum = FMath::RandRange(0, 1);

		if (RandonNum == 0)
		{
			OwningCombatCharacter->ToggleCrouch();
		}

		StayCombatAlert = true;
		UpdatCombatAlert();

		GetWorldTimerManager().ClearTimer(THandler_MoveToNearbyDestination);
	}
}



void ACombatAIController::StartPatrol()
{
	if (!PatrolFollowerComponent || EnemyActor || LastSeenEnemyActor || CurrentBehaviourState == AIBehaviourState::Patrol) {
		GetWorldTimerManager().ClearTimer(THandler_PatrolStart);
		return;
	}

	auto OutLocation = PatrolFollowerComponent->GetCurrentPathPoint();

	if (!OutLocation.IsZero())
	{
		TargetDestination = OutLocation;
		auto Movement = AIMovementComponent->MoveToDestination(TargetDestination, 0.f, AIBehaviourState::Patrol, false);

		if (Movement == EPathFollowingRequestResult::RequestSuccessful)
		{
			SetBehaviourState(AIBehaviourState::Patrol);
			GetWorldTimerManager().ClearTimer(THandler_MountedGun);
			GetWorldTimerManager().ClearTimer(THandler_PatrolStart);
		}
	}
}

void ACombatAIController::MoveToNextPatrolPoint()
{
	auto OutLocation = PatrolFollowerComponent->GetNextPathPoint();

	if (!OutLocation.IsZero())
	{
		TargetDestination = OutLocation;
		AIMovementComponent->MoveToDestination(TargetDestination, 0.f, AIBehaviourState::Patrol, false);
	}

}

void ACombatAIController::CheckCommanderOrder()
{
	Commander = OwningCombatCharacter->getCommander();

	if (Commander == nullptr) {
		return;
	}

	// Assign Order Event
	if (!HasAssignedOrderEvent) {
		Commander->OnOrderSent.AddDynamic(this, &ACombatAIController::OnOrderReceived);
		OwningCombatCharacter->DropMountedGun();
		HasAssignedOrderEvent = true;
		StayCombatAlert = false; // refresh state of behaviour
		SetBehaviourState(AIBehaviourState::PriorityOrdersCommander);
	}


	if (CurrentCommand == CommanderOrders::Follow)
	{
		HasChosenNearTargetDest = true;
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



		if (!IsNearCommander())
		{
			OwningCombatCharacter->DropMountedGun();
			TargetDestination = Commander->GetActorLocation();
			AIMovementComponent->MoveToDestination(TargetDestination, AcceptanceRadius, AIBehaviourState::PriorityOrdersCommander);
		}
		else
		{
			FindMountedGun();
		}

	}
}


void ACombatAIController::TargetFound()
{
	if (!HasPlayedTargetFoundSound)
	{
		OwningCombatCharacter->PlayVoiceSound(OwningCombatCharacter->GetVoiceClipsSet()->TargetFoundSound);
		HasPlayedTargetFoundSound = true;
	}
}

bool ACombatAIController::IsNearCommander()
{
	if (Commander && OwningCombatCharacter->GetDistanceTo(Commander) <= AcceptanceRadius) {
		return true;
	}
	return false;
}

bool ACombatAIController::IsNearCommander(FVector Location)
{
	if (Commander && FVector::Distance(Commander->GetActorLocation(), Location) <= AcceptanceRadius) {
		return true;
	}
	return false;
}

void ACombatAIController::SetBehaviourState(AIBehaviourState State)
{
	if (CurrentBehaviourState == AIBehaviourState::PriorityOrdersCommander) {
		return;
	}

	CurrentBehaviourState = State;
}
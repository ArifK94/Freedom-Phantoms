#include "Controllers/CombatAIController.h"

#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/PumpActionWeapon.h"
#include "Weapons/MountedGun.h"
#include "Props/Stronghold.h"
#include "CustomComponents/AIMovementComponent.h"
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
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "..\..\Public\Controllers\CombatAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	OwningCombatCharacter->DropMountedGun();

	float TargetRadius = AcceptanceRadius;

	CurrentCommand = RecruitInfo->CurrentCommand;
	TargetDestination = RecruitInfo->TargetLocation;

	// Defending point should be right where it was ordered to go to
	if (CurrentCommand == CommanderOrders::Defend)
	{
		TargetRadius = 0.0f;
	}

	CanFindCover = true;
	HasChosenNearTargetDest = false;


	StayCombatAlert = false;
	UpdatCombatAlert();

	GetWorldTimerManager().SetTimer(THandler_MoveToNearbyDestination, this, &ACombatAIController::MoveToRandomPoint, .5f, true);

	MoveToTarget(TargetRadius);
}

ACombatAIController::ACombatAIController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	AcceptanceRadius = 300.0f;
	DistanceDiffSprint = 200.0f;
	TargetSightRadius = 7000.0f;
	MountedGunSightRadius = 500.0f;

	MovementDebugSphereRadius = 10.0f;
	MovementDebugLifetTime = 1.0f;

	TimeBetweenShotsMin = 2.0f;
	TimeBetweenShotsMax = 3.0f;

	DestinationRadius = 300.0f;

	AIMovementComponent = CreateDefaultSubobject<UAIMovementComponent>(TEXT("AIMovementComponent"));

	CoverFinderComponent = CreateDefaultSubobject<UCoverFinderComponent>(TEXT("CoverFinderComponent"));

	TargetFinderComponent = CreateDefaultSubobject<UTargetFinderComponent>(TEXT("TargetFinderComponent"));

	MountedGunFinderComponent = CreateDefaultSubobject<UMountedGunFinderComponent>(TEXT("MountedGunFinderComponent"));
}

void ACombatAIController::Init()
{
	if (OwningCombatCharacter == nullptr) {
		return;
	}

	OwningCombatCharacter->GetCharacterMovement()->bUseRVOAvoidance = false;
	OwningCombatCharacter->SetUseAimCameraSpring(false);

	PerceptionComp = Cast<UAIPerceptionComponent>(OwningCombatCharacter->GetComponentByClass(UAIPerceptionComponent::StaticClass()));

	// Get AI Sight Config
	//UAISenseConfig* SightConfig = GetPerceptionSenseConfig(UAISense_Sight::StaticClass());
	//if (SightConfig)
	//{
	//	AISightConfig = Cast<UAISenseConfig_Sight>(SightConfig);
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("SetSightRange: Config == nullptr"));
	//}

	HasAssignedOrderEvent = false;

	// if assigned an MG at the beginning
	if (OwningCombatCharacter->GetMountedGun())
	{
		TargetDestination = OwningCombatCharacter->GetMountedGun()->GetActorLocation();
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
	if (AISightConfig == nullptr) {
		return;
	}

	// Set Vision angle based whether character is in the helicopter
	if (OwningCombatCharacter->GetIsInAircraft())
	{
		AISightConfig->PeripheralVisionAngleDegrees = 90.0f;
	}
	else
	{
		AISightConfig->PeripheralVisionAngleDegrees = 180.0f;
	}

	PerceptionComp->RequestStimuliListenerUpdate();

}

EPathFollowingRequestResult::Type ACombatAIController::MoveToTarget(float AcceptRadius, bool WalkNearTarget)
{
	if (TargetDestination.IsZero() || OwningCombatCharacter->GetIsInAircraft() || OwningCombatCharacter->IsUsingMountedWeapon()) {
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

void ACombatAIController::BeginPlay()
{
	Super::BeginPlay();

	DefaultDestinationRadius = DestinationRadius;

	Init();
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get owning character
	OwningCombatCharacter = Cast<ACombatCharacter>(InPawn);

	CanFindCover = true;

	if (OwningCombatCharacter)
	{
		// Attach Follow Camera to head socket
		OwningCombatCharacter->FollowCamera->AttachToComponent(OwningCombatCharacter->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, OwningCombatCharacter->GetHeadSocket());

		PumpActionWeapon = Cast<APumpActionWeapon>(OwningCombatCharacter->GetPrimaryWeapon());

		GetWorldTimerManager().SetTimer(THandler_FindEnemy, this, &ACombatAIController::FindEnemy, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_EndFire, this, &ACombatAIController::EndFiring, FMath::RandRange(TimeBetweenShotsMin, TimeBetweenShotsMax), true);
		GetWorldTimerManager().SetTimer(THandler_MountedGun, this, &ACombatAIController::FindMountedGun, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_CommanderOrders, this, &ACombatAIController::CheckCommanderOrder, 1.0f, true);
		//GetWorldTimerManager().SetTimer(THandler_BeginPeakCover, this, &ACombatAIController::BeginCoverPeak, FMath::RandRange(2.0f, 5.0f), true);

		OwningCombatCharacter->OnRappelUpdate.AddDynamic(this, &ACombatAIController::OnRappelUpdated);

		UHealthComponent* HealthComp = OwningCombatCharacter->GetHealthComp();
		HealthComp->SetRegenerateHealth(true);
		HealthComp->OnHealthChanged.AddDynamic(this, &ACombatAIController::OnHealthChanged);
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
		OwningCombatCharacter->OnRappelUpdate.RemoveDynamic(this, &ACombatAIController::OnRappelUpdated);

		UHealthComponent* HealthComp = OwningCombatCharacter->GetHealthComp();
		HealthComp->OnHealthChanged.RemoveDynamic(this, &ACombatAIController::OnHealthChanged);
	}


	ClearTimers();
}

void ACombatAIController::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo)
{
	if (Health <= 0.0f)
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

void ACombatAIController::ClearTimers()
{
	GetWorldTimerManager().ClearTimer(THandler_ShootEnemy);
	GetWorldTimerManager().ClearTimer(THandler_EndFire);
	GetWorldTimerManager().ClearTimer(THandler_MountedGun);
	GetWorldTimerManager().ClearTimer(THandler_FindEnemy);
	GetWorldTimerManager().ClearTimer(THandler_CommanderOrders);
	GetWorldTimerManager().ClearTimer(THandler_FindCover);
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

/// <summary>
/// TODO: Add limit to the number of characters to be processed so we do not have like 20 enemies to choose from
/// </summary>
void ACombatAIController::FindEnemy()
{
	if (TargetFinderComponent == nullptr) {
		return;
	}


	AActor* ChosenTarget = TargetFinderComponent->FindTarget();


	if (ChosenTarget) // found an enemy?
	{

		if (OwningCombatCharacter->IsTakingCover())
		{
			if (TargetFinderComponent->IsTargetBehind(OwningCombatCharacter, ChosenTarget))
			{
				OwningCombatCharacter->StopCover();
			}
		}


		if (OwningCombatCharacter->IsUsingMountedWeapon() && OwningCombatCharacter->GetMountedGun())
		{
			FVector Start = ChosenTarget->GetActorLocation() - OwningCombatCharacter->GetActorLocation();
			Start = UKismetMathLibrary::InverseTransformDirection(OwningCombatCharacter->FollowCamera->GetComponentTransform(), Start);
			FRotator TargetRot = UKismetMathLibrary::MakeRotFromX(Start);

			bool YawRange = UKismetMathLibrary::InRange_FloatFloat(TargetRot.Yaw, OwningCombatCharacter->GetMountedGun()->GetYawMin(), OwningCombatCharacter->GetMountedGun()->GetYawMax(), false, false);
			bool PitchRange = UKismetMathLibrary::InRange_FloatFloat(TargetRot.Pitch, OwningCombatCharacter->GetMountedGun()->GetPitchMin(), OwningCombatCharacter->GetMountedGun()->GetPitchMax(), false, false);

			// check if enemy position is NOT within turret's pitch and yaw boundaries or is not behind the MG
			if (!YawRange || !PitchRange || TargetFinderComponent->IsTargetBehind(OwningCombatCharacter, ChosenTarget))
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

		GetWorldTimerManager().ClearTimer(THandler_LastSeenEnemy);
		EnemyActor = ChosenTarget;
		SetFocus(ChosenTarget); // face in the direction of the enemy

		// Being firing at enemy
		if (!THandler_ShootEnemy.IsValid()) {
			GetWorldTimerManager().SetTimer(THandler_ShootEnemy, this, &ACombatAIController::ShootAtEnemy, 1.0f, true);
		}

		//if (!THandler_FindCover.IsValid()) {
		//	GetWorldTimerManager().SetTimer(THandler_FindCover, this, &ACombatAIController::MoveToCover, FMath::RandRange(2.f, 5.f), true);
		//}

		// Do find a random point if current command is defend
		// & is set to use the mounted gun
		if (!THandler_MoveToNearbyDestination.IsValid() && CurrentCommand != CommanderOrders::Defend && !OwningCombatCharacter->GetMountedGun()) {
			HasChosenNearTargetDest = false;
			TargetDestination = OwningCombatCharacter->GetActorLocation();
			GetWorldTimerManager().SetTimer(THandler_MoveToNearbyDestination, this, &ACombatAIController::MoveToRandomPoint, FMath::RandRange(.5f, 2.f), true);
		}


	}
	else  // not found an enemy
	{
		if (OwningCombatCharacter->IsUsingMountedWeapon())
		{
			OwningCombatCharacter->GetMountedGun()->ResetCamera();
		}

		if (EnemyActor) // if previous enemy still exists, look at the last location it was seen
		{
			LastSeenEnemyActor = EnemyActor;
			LastSeenPosition = LastSeenEnemyActor->GetActorLocation();
			EnemyActor = nullptr;
		}
	}
}

void ACombatAIController::UpdateLastSeen()
{
	SetFocalPoint(LastSeenPosition);

	// look straight ahead with the MG direction
	if (OwningCombatCharacter->IsUsingMountedWeapon() && OwningCombatCharacter->GetMountedGun() && OwningCombatCharacter->GetMountedGun()->GetAdjustBehindMG())
	{
		OwningCombatCharacter->GetCapsuleComponent()->SetWorldRotation(OwningCombatCharacter->GetMountedGun()->GetCharacterStandRot());
	}

	if (CurrentWeapon->getCurrentAmmo() <= 0 || CurrentWeapon->getCurrentAmmo() < CurrentWeapon->getAmmoPerClip()) // reload clip if finished completely or  reload if not on full clip
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

	// Stop aiming if not combat alert
	if (!StayCombatAlert)
	{
		OwningCombatCharacter->EndAim();
	}

	LastSeenEnemyActor = nullptr;
	GetWorldTimerManager().ClearTimer(THandler_LastSeenEnemy);
}

void ACombatAIController::ShootAtEnemy()
{
	CurrentWeapon = OwningCombatCharacter->GetCurrentWeapon();

	if (CurrentWeapon == nullptr) {
		return;
	}

	if (OwningCombatCharacter->IsRepellingDown() || OwningCombatCharacter->IsSwappingWeapon() || OwningCombatCharacter->IsReloading())
	{
		/**
		* Remove this if statement when the end reload is triggered as soon as the capacity is reached. Problem arises when this function is returned because the reload was not being ended.
		*/
		if (OwningCombatCharacter->IsReloading())
		{
			if (CurrentWeapon->getCurrentAmmo() >= CurrentWeapon->getAmmoPerClip())
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
		OwningCombatCharacter->GetCurrentWeapon()->SetUnlimitedAmmo(true);

	if (EnemyActor)
	{
		OwningCombatCharacter->BeginAim();

		// if using a mounted gun
		if (OwningCombatCharacter->GetMountedGun())
		{
			FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(OwningCombatCharacter->GetActorLocation(), EnemyActor->GetActorLocation());
			OwningCombatCharacter->GetMountedGun()->SetRotatioInput(TargetRot);
		}

		if (CurrentWeapon->getCurrentAmmo() <= 0 || OwningCombatCharacter->IsReloading())
		{
			ReloadWeapon();
		}
		else
		{
			// check if enemy distance is close, if so then pull out pistol
			float DistanceDiff = FVector::Dist(OwningCombatCharacter->GetActorLocation(), EnemyActor->GetActorLocation());
			float randomDistanceLimit = FMath::RandRange(500.0f, 1000.0f);
			bool IsTargetClose = DistanceDiff < randomDistanceLimit;

			if (!IsTargetClose && CurrentWeapon == OwningCombatCharacter->GetSecondaryWeaponObj() // if target is not close, then switch back to primary
				&& !OwningCombatCharacter->GetIsInAircraft())
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

		// Focus on the last seen enemy
		if (!THandler_LastSeenEnemy.IsValid()) {
			OwningCombatCharacter->BeginAim();
			GetWorldTimerManager().SetTimer(THandler_LastSeenEnemy, this, &ACombatAIController::UpdateLastSeen, 1.0f, false, FMath::RandRange(2.f, 5.f));
		}

		GetWorldTimerManager().ClearTimer(THandler_ShootEnemy);
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

	if (CurrentWeapon->getCurrentAmmo() <= 0 && !PumpActionWeapon) // Reload the weapon
	{
		if (IsTargetClose &&
			CurrentWeapon != OwningCombatCharacter->GetSecondaryWeaponObj() && // if using primary weapon & enemy is nearby, then swap to secondary
			!OwningCombatCharacter->GetIsInAircraft())
		{
			OwningCombatCharacter->BeginWeaponSwap();
		}
		else
		{
			OwningCombatCharacter->BeginReload();
		}
	}
	else if (PumpActionWeapon && (CurrentWeapon->getCurrentAmmo() <= 0 || OwningCombatCharacter->IsReloading())) // pump action weapons can be fired as soon as the first ammo shell is inserted, so better to add few more bullets first before firing again
	{
		int RandomAmount = rand() % CurrentWeapon->getAmmoPerClip();

		if (CurrentWeapon->getCurrentAmmo() <= 0 || CurrentWeapon->getCurrentAmmo() < RandomAmount)
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
	if (OwningCombatCharacter->GetMountedGun() && !OwningCombatCharacter->GetMountedGun()->GetCanExitMG())
	{
		// if using an aircraft MG for instance, which should not be exited
		GetWorldTimerManager().ClearTimer(THandler_MountedGun);
		return;
	}

	// If an MG has been assigned
	if (OwningCombatCharacter->GetMountedGun())
	{
		if (!OwningCombatCharacter->IsReloading() && !OwningCombatCharacter->IsUsingMountedWeapon())
		{
			OwningCombatCharacter->UseMountedGun();
			return;
		}

		// in case player or another NPC has reached the MG before AI
		if (OwningCombatCharacter->GetMountedGun()->GetOwner() != OwningCombatCharacter)
		{
			OwningCombatCharacter->DropMountedGun();
			CanFindCover = true;
		}
		else if (TargetFinderComponent->IsTargetBehind(OwningCombatCharacter, EnemyActor))
		{
			OwningCombatCharacter->DropMountedGun(false);
			CanFindCover = true;
		}
		else // if MG is free, keep moving towards it
		{
			MoveToTarget(0.0f, false);
			CanFindCover = false;
			return;
		}
	}

	if (MountedGunFinderComponent)
	{
		auto SelectedMG = MountedGunFinderComponent->FindMG();

		// if found an MG 
		// & enemy is not behind the MG
		if (SelectedMG && !TargetFinderComponent->IsTargetBehind(SelectedMG, EnemyActor))
		{
			SelectedMG->SetPotentialOwner(OwningCombatCharacter);
			OwningCombatCharacter->SetMountedGun(SelectedMG);
			CanFindCover = false;

			TargetDestination = OwningCombatCharacter->GetMountedGun()->GetCharacterStandPos();
			MoveToTarget(0.0f, false);
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

	auto Movement = MoveToTarget(0.0f);

	// Find a random point around destination if has arrived at the original target destination
	if (!HasChosenNearTargetDest && Movement == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		auto NearDestination = FindNearbyDestinationPoint();

		if (!NearDestination.IsZero())
		{
			TargetDestination = NearDestination; // upate the target destination
			Movement = MoveToTarget(0.0f); // Move to new destination
			HasChosenNearTargetDest = true;
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

FVector ACombatAIController::FindNearbyDestinationPoint()
{
	// Assign default target by the destination set
	FVector TargetDest = TargetDestination;

	// create a collision sphere
	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(DestinationRadius);

	// create tarray for hit results
	TArray<FHitResult> OutHits;

	// check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, TargetDest, TargetDest, FQuat::Identity, ECC_Visibility, MyColSphere);


	if (isHit)
	{
		// Is there already a character in that target dest?
		bool IsPointTaken = false;
		for (auto& Hit : OutHits)
		{
			AActor* DamagedActor = Hit.GetActor();
			if (DamagedActor)
			{
				auto HitCharacter = Cast<ABaseCharacter>(DamagedActor);
				if (HitCharacter && HitCharacter->GetHealthComp()->IsAlive())
				{
					// if the character hit is the commander then this hit point location should be available
					if (Commander)
					{
						if (HitCharacter != Commander)
						{
							IsPointTaken = true;
						}
					}
					else
					{
						IsPointTaken = true;
					}

				}
			}
		}

		if (IsPointTaken)
		{
			FNavLocation NavLocation;
			UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
			bool navResult = NavigationArea->GetRandomReachablePointInRadius(TargetDestination, DestinationRadius, NavLocation);

			if (navResult)
			{
				TargetDest = NavLocation.Location;
			}

		}
	}



	return TargetDest;
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
	}


	if (CurrentCommand == CommanderOrders::Follow)
	{
		TargetDestination = Commander->GetActorLocation();
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

		auto CurrentMovement = MoveToTarget(AcceptanceRadius);

		if (CurrentMovement == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			FindMountedGun();
		}
		else
		{
			// exit from MG if currently using or if an MG has been assigned
			OwningCombatCharacter->DropMountedGun();
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
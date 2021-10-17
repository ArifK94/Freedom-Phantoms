#include "Controllers/CombatAIController.h"

#include "Managers/GameModeManager.h"
#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/PumpActionWeapon.h"
#include "Weapons/MountedGun.h"
#include "Props/Stronghold.h"
#include "CustomComponents/CoverPointComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
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
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"

void ACombatAIController::OnOrderReceived(UCommanderRecruit* RecruitInfo)
{
	if (OwningCombatCharacter->IsTakingCover())
	{
		OwningCombatCharacter->StopCover();
	}

	OwningCombatCharacter->DropMountedGun();
	StayCombatAlert = true;

	float TargetRadius = AcceptanceRadius;

	CurrentCommand = RecruitInfo->CurrentCommand;
	TargetDestination = RecruitInfo->TargetLocation;

	switch (CurrentCommand)
	{
	case CommanderOrders::Defend:
		TargetRadius = 0.0f;
		break;
	}

	CanFindCover = true;


	MoveToTarget(TargetRadius);
}

ACombatAIController::ACombatAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	AcceptanceRadius = 300.0f;
	DistanceDiffSprint = 200.0f;
	CoverRadius = 1000.0f;
	NumberOfCoverTraces = 35.0f;
	TargetSightRadius = 7000.0f;
	MountedGunSightRadius = 500.0f;

	MovementDebugSphereRadius = 10.0f;
	MovementDebugLifetTime = 1.0f;

	TimeBetweenShotsMin = 2.0f;
	TimeBetweenShotsMax = 3.0f;

	ResetMovementCountdown = 5.0f;

	LastSeenDuration = 5.0f;
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
	UAISenseConfig* SightConfig = GetPerceptionSenseConfig(UAISense_Sight::StaticClass());
	if (SightConfig)
	{
		AISightConfig = Cast<UAISenseConfig_Sight>(SightConfig);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetSightRange: Config == nullptr"));
	}


	// Alternative to AI Sight Perception in case 360 sight is wanted
	if (TargetSightSphere == nullptr)
	{
		TargetSightSphere = NewObject<USphereComponent>(OwningCombatCharacter);
		if (TargetSightSphere)
		{
			TargetSightSphere->RegisterComponent();
			TargetSightSphere->AttachToComponent(OwningCombatCharacter->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			TargetSightSphere->SetSphereRadius(TargetSightRadius);
			TargetSightSphere->SetCollisionProfileName(TEXT("AITargetSight"));
		}
	}

	HasAssignedOrderEvent = false;

	// if assigned an MG at the beginning
	if (OwningCombatCharacter->GetMountedGun())
	{
		CurrentMovement = EPathFollowingRequestResult::AlreadyAtGoal;
		TargetDestination = OwningCombatCharacter->GetMountedGun()->GetActorLocation();
	}

	UHealthComponent* HealthComp = OwningCombatCharacter->GetHealthComp();
	HealthComp->SetRegenerateHealth(true);
	HealthComp->OnHealthChanged.AddDynamic(this, &ACombatAIController::OnHealthChanged);
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

void ACombatAIController::MoveToTarget(float AcceptRadius, bool WalkNearTarget)
{
	if (OwningCombatCharacter->GetIsInAircraft()) {
		return;
	}

	CurrentMovement = MoveToLocation(TargetDestination, AcceptRadius, StopOnOverlap, UsePathfinding, ProjectDestinationToNavigation, CanStrafe, FilterClass, AllowPartialPaths);

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
}

void ACombatAIController::BeginPlay()
{
	Super::BeginPlay();
	CurrentResetMovementCountdown = ResetMovementCountdown;

	GameModeManager = Cast<AGameModeManager>(GetWorld()->GetAuthGameMode());

	StayCombatAlert = false;

	LastSeenTimeCurrent = LastSeenDuration;
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
		OwningCombatCharacter->FollowCamera->AttachToComponent(OwningCombatCharacter->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, OwningCombatCharacter->GetHeadSocket());

		GetWorldTimerManager().SetTimer(THandler_FindEnemy, this, &ACombatAIController::FindEnemy, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_LastSeenEnemy, this, &ACombatAIController::UpdateLastSeen, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_BeginFire, this, &ACombatAIController::ShootAtEnemy, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_EndFire, this, &ACombatAIController::EndFiring, FMath::RandRange(TimeBetweenShotsMin, TimeBetweenShotsMax), true);
		GetWorldTimerManager().SetTimer(THandler_MountedGun, this, &ACombatAIController::FindMountedGun, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_CommanderOrders, this, &ACombatAIController::CheckCommanderOrder, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_Sprint, this, &ACombatAIController::UpdateSprint, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_CombatAlert, this, &ACombatAIController::UpdatCombatAlert, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_FindCover, this, &ACombatAIController::FindCover, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_BeginPeakCover, this, &ACombatAIController::BeginCoverPeak, FMath::RandRange(2.0f, 5.0f), true);
		//GetWorldTimerManager().SetTimer(THandler_ResetMovement, this, &ACombatAIController::ResetLocation, 2.0f, true);

		PumpActionWeapon = Cast<APumpActionWeapon>(OwningCombatCharacter->GetPrimaryWeapon());
	}

	// run behavior tree if specified
	if (BTAsset)
	{
		AAIController::RunBehaviorTree(BTAsset);
	}
}

void ACombatAIController::OnUnPossess()
{
	ClearTimers();

	Super::OnUnPossess();
}

void ACombatAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OwningCombatCharacter)
	{
		if (PerceptionComp != nullptr)
		{
			SetVisionAngle();
		}
	}
	else
	{
		ClearTimers();

		AActor* OwningCharacter = GetOwner();

		if (OwningCharacter)
		{
			OwningCharacter->Destroy();
		}
	}
}

void ACombatAIController::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo)
{
	if (Health <= 0.0f)
	{
		ClearTimers();

		OwningCombatCharacter->DetachFromControllerPendingDestroy();
	}
}


void ACombatAIController::ClearTimers()
{
	GetWorldTimerManager().ClearTimer(THandler_BeginFire);
	GetWorldTimerManager().ClearTimer(THandler_EndFire);
	GetWorldTimerManager().ClearTimer(THandler_MountedGun);
	GetWorldTimerManager().ClearTimer(THandler_FindEnemy);
	GetWorldTimerManager().ClearTimer(THandler_CommanderOrders);
	GetWorldTimerManager().ClearTimer(THandler_Sprint);
	GetWorldTimerManager().ClearTimer(THandler_CombatAlert);
	GetWorldTimerManager().ClearTimer(THandler_FindCover);
}


void ACombatAIController::UpdateSprint()
{
	//FVector OwnerLocation = OwningCombatCharacter->GetActorLocation();

	//float CurrentTargetDistance = UKismetMathLibrary::Vector_Distance(OwnerLocation, TargetDestination);

	//if (CurrentTargetDistance > AcceptanceRadius)
	//{
	//	OwningCombatCharacter->BeginSprint();
	//}
	//else
	//{
	//	OwningCombatCharacter->EndSprint();
	//}
}

void ACombatAIController::UpdatCombatAlert()
{
	if (OwningCombatCharacter->IsFiring() || LastSeenEnemyActor || EnemyActor)
	{
		StayCombatAlert = true;
	}
	else
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

void ACombatAIController::FindEnemy()
{
	AActor* ChosenTarget = nullptr;
	TArray<AActor*> ActorsInSight;
	float TargetSightDistance = TargetSightRadius;

	if (TargetSightSphere)
	{
		TargetSightSphere->GetOverlappingActors(ActorsInSight, ABaseCharacter::StaticClass());
	}
	//else if (OwningCombatCharacter->GetIsInAircraft())
	//{
	//	PerceptionComp->GetKnownPerceivedActors(TSubclassOf<UAISense_Sight>(), ActorsInSight);
	//}
	else
	{
		return;
	}

	FVector OwnerLocation = OwningCombatCharacter->GetActorLocation();

	for (int index = 0; index < ActorsInSight.Num(); index++)
	{
		AActor* PotentialEnemy = ActorsInSight[index];

		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(PotentialEnemy->GetComponentByClass(UHealthComponent::StaticClass()));

		if (CurrentHealth && CurrentHealth->IsAlive() && CurrentHealth->GetSelectedFaction() != TeamFaction::Neutral)
		{
			bool IsEnemy = !UHealthComponent::IsFriendly(OwningCombatCharacter, PotentialEnemy);

			// is target alive & an enemy
			if (IsEnemy)
			{
				FVector EnemyLocation = PotentialEnemy->GetActorLocation();


				// check if can see the target
				FHitResult HitTargetResult;

				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(OwningCombatCharacter);
				QueryParams.bTraceComplex = false;

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
							ChosenTarget = PotentialEnemy;
						}
					}
				}
			}
		}
	}

	if (ChosenTarget)
	{
		if (OwningCombatCharacter->IsUsingMountedWeapon())
		{
			FVector Start = ChosenTarget->GetActorLocation() - OwningCombatCharacter->GetActorLocation();
			Start = UKismetMathLibrary::InverseTransformDirection(OwningCombatCharacter->FollowCamera->GetComponentTransform(), Start);
			FRotator TargetRot = UKismetMathLibrary::MakeRotFromX(Start);

			bool YawRange = UKismetMathLibrary::InRange_FloatFloat(TargetRot.Yaw, OwningCombatCharacter->GetMountedGun()->GetYawMin(), OwningCombatCharacter->GetMountedGun()->GetYawMax(), false, false);
			bool PitchRange = UKismetMathLibrary::InRange_FloatFloat(TargetRot.Pitch, OwningCombatCharacter->GetMountedGun()->GetPitchMin(), OwningCombatCharacter->GetMountedGun()->GetPitchMax(), false, false);

			// check if enemy position is NOT within turret's pitch and yaw boundaries and is not behind the MG
			if (!YawRange || !PitchRange || IsEnemyBehindMG(ChosenTarget))
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
	}
	else
	{
		if (OwningCombatCharacter->IsUsingMountedWeapon())
		{
			OwningCombatCharacter->GetMountedGun()->ResetCamera();
		}
	}

	if (ChosenTarget) // if found an enemy
	{
		EnemyActor = ChosenTarget;
		SetFocus(EnemyActor); // face in the direction of the enemy
	}
	else // not found an enemy
	{
		if (EnemyActor) // if previous enemy still exists, look at the last location it was seen
		{
			LastSeenTimeCurrent = LastSeenDuration; // update the duration countdown
			LastSeenEnemyActor = EnemyActor;
			LastSeenPosition = LastSeenEnemyActor->GetActorLocation();
			EnemyActor = nullptr;
		}
	}


	if (EnemyActor) {
		OwningCombatCharacter->TargetFound();
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);
	}

}

void ACombatAIController::UpdateLastSeen()
{
	if (LastSeenEnemyActor == nullptr || LastSeenTimeCurrent <= 0.0f) {
		return;
	}

	SetFocalPoint(LastSeenPosition);

	// Countdown to forget
	LastSeenTimeCurrent--;

	if (LastSeenTimeCurrent <= 0.0f)
	{
		LastSeenEnemyActor = nullptr;
	}

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
	if (OwningCombatCharacter->IsUsingMountedWeapon())
	{
		// if using an aircraft MG for instance, which should not be exited 
		if (!OwningCombatCharacter->GetMountedGun()->GetCanExitMG())
		{
			return;
		}

		// is enemy beind the mounted gun? then drop the MG
		if (IsEnemyBehindMG() || CurrentMovement != EPathFollowingRequestResult::AlreadyAtGoal)
		{
			OwningCombatCharacter->DropMountedGun();
		}

		// a player can use the MG at the last second which results in more than one actor using the MG
		if (OwningCombatCharacter->GetMountedGun()->GetPotentialOwner() != OwningCombatCharacter ||
			OwningCombatCharacter->GetMountedGun()->GetOwner() != OwningCombatCharacter)
		{
			OwningCombatCharacter->SetMountedGun(nullptr);
			CanFindCover = true;
		}


		return;
	}

	// If an MG has been assigned
	if (OwningCombatCharacter->GetMountedGun())
	{
		if (CurrentMovement == EPathFollowingRequestResult::AlreadyAtGoal && !IsEnemyBehindMG() && !OwningCombatCharacter->IsReloading())
		{
			OwningCombatCharacter->UseMountedGun();
			return;
		}

		// in case player or another NPC has reached the MG before AI
		if (OwningCombatCharacter->GetMountedGun()->GetPotentialOwner() != OwningCombatCharacter ||
			OwningCombatCharacter->GetMountedGun()->GetOwner() != OwningCombatCharacter)
		{
			OwningCombatCharacter->SetMountedGun(nullptr);
			CanFindCover = true;
		}
		else // if MG is free, keep moving towards it
		{
			MoveToTarget(0.0f, false);
			CanFindCover = false;
			return;
		}
	}

	AMountedGun* SelectedMG = nullptr;
	float TargetSightDistance = MountedGunSightRadius;

	// give priority to stronghold MG for defensive positions
	if (CurrentStronghold)
	{
		TArray<AActor*> ChildActors;
		CurrentStronghold->GetAllChildActors(ChildActors);

		for (int i = 0; i < ChildActors.Num(); i++)
		{
			AMountedGun* PotentialMG = Cast<AMountedGun>(ChildActors[i]);

			if (PotentialMG)
			{
				bool HasNoOwner = PotentialMG->GetOwner() == nullptr && PotentialMG->GetPotentialOwner() == nullptr;
				bool IsSamePotentialOwner = PotentialMG->GetPotentialOwner() != nullptr && PotentialMG->GetPotentialOwner() == OwningCombatCharacter;

				if (HasNoOwner || IsSamePotentialOwner && PotentialMG->GetCanTraceInteraction())
				{
					FVector MGLocation = PotentialMG->GetActorLocation();

					float DistanceDiff = FVector::Dist(OwningCombatCharacter->GetActorLocation(), MGLocation);

					if (DistanceDiff < TargetSightDistance)
					{
						TargetSightDistance = DistanceDiff;
						SelectedMG = PotentialMG;
					}
				}
			}
		}
	}
	else // if not guarding a stronghold and freely out in the open
	{
		// create a collision sphere
		FCollisionShape MyColSphere = FCollisionShape::MakeSphere(MountedGunSightRadius);

		// create tarray for hit results
		TArray<FHitResult> OutHits;

		// check if something got hit in the sweep
		bool isHit = GetWorld()->SweepMultiByChannel(OutHits, OwningCombatCharacter->GetActorLocation(), OwningCombatCharacter->GetActorLocation(), FQuat::Identity, ECC_Visibility, MyColSphere);

		if (isHit)
		{
			// loop through TArray
			for (auto& Hit : OutHits)
			{
				AActor* HitActor = Hit.GetActor();

				if (HitActor)
				{
					AMountedGun* PotentialMG = Cast<AMountedGun>(HitActor);

					if (PotentialMG)
					{
						// check if mounted gun is present in the stronghold and has no owner as well as no potential owner in case another AI wishes to use it
						bool HasNoOwner = PotentialMG->GetOwner() == nullptr && PotentialMG->GetPotentialOwner() == nullptr;
						bool IsSamePotentialOwner = PotentialMG->GetPotentialOwner() != nullptr && PotentialMG->GetPotentialOwner() == OwningCombatCharacter;

						if (HasNoOwner || IsSamePotentialOwner && PotentialMG->GetCanTraceInteraction())
						{
							FVector MGLocation = PotentialMG->GetActorLocation();

							float DistanceDiff = FVector::Dist(OwningCombatCharacter->GetActorLocation(), MGLocation);

							if (DistanceDiff < TargetSightDistance)
							{
								TargetSightDistance = DistanceDiff;
								SelectedMG = PotentialMG;
							}
						}
					}
				}

			}
		}

	}

	OwningCombatCharacter->SetMountedGun(SelectedMG);

	if (OwningCombatCharacter->GetMountedGun())
	{
		OwningCombatCharacter->GetMountedGun()->SetPotentialOwner(OwningCombatCharacter);
		CanFindCover = false;

		TargetDestination = OwningCombatCharacter->GetMountedGun()->GetCharacterStandPos();
		MoveToTarget(0.0f, false);
	}

}

void ACombatAIController::FindCover()
{
	if (!CanFindCover) {
		return;
	}

	if (!HasChosenCover)
	{
		if (EnemyActor)
		{
			GenerateCoverPoints(EnemyActor);
		}
		else
		{
			if (CurrentStronghold)
			{
				ChosenCoverPointComponent = CurrentStronghold->GetCoverPoint(OwningCombatCharacter);

				if (ChosenCoverPointComponent)
				{
					TargetDestination = ChosenCoverPointComponent->GetComponentLocation();
					CoverLocationPoints.Add(TargetDestination);
					HasChosenCover = true;
				}
			}
		}
	}


	// check if current cover has been taken,
	// if so, then find another cover point
	//FWorldCoverPoint CoverLocation = FWorldCoverPoint();
	//CoverLocation.Location = TargetDestination;
	//CoverLocation.Owner = OwningCombatCharacter;

	//if (GameModeManager->IsCoverPointTaken(CoverLocation))
	//{
	//	HasChosenCover = false;
	//}

	// Update nav movement
	if (HasChosenCover)
	{
		MoveToTarget(0.0f);

		TakeCover();
	}

	if (OwningCombatCharacter->IsTakingCover())
	{
		if (IsEnemyBehindMG(EnemyActor))
		{
			OwningCombatCharacter->StopCover();
		}
	}
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

	if (Commander == nullptr) {
		return;
	}

	// Assign Order Event
	if (!HasAssignedOrderEvent) {
		Commander->OnOrderSent.AddDynamic(this, &ACombatAIController::OnOrderReceived);
		OwningCombatCharacter->DropMountedGun();
		HasAssignedOrderEvent = true;
	}


	StayCombatAlert = true;

	if (CurrentCommand == CommanderOrders::Follow)
	{
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

		MoveToTarget(AcceptanceRadius);

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

void ACombatAIController::ResetLocation()
{
	if (OwningCombatCharacter->GetIsInAircraft()) {
		return;
	}

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

bool ACombatAIController::IsEnemyBehindMG(AActor* Enemy)
{
	if (OwningCombatCharacter->GetMountedGun() == nullptr) {
		return false;
	}

	if (Enemy == nullptr) {
		Enemy = EnemyActor;
	}

	if (Enemy)
	{
		FVector MGForwardPos = UKismetMathLibrary::GetForwardVector(OwningCombatCharacter->GetMountedGun()->GetActorRotation());
		FVector Normalised = Enemy->GetActorLocation() - OwningCombatCharacter->GetMountedGun()->GetActorLocation();
		UKismetMathLibrary::Vector_Normalize(Normalised);
		float Angle = UKismetMathLibrary::Dot_VectorVector(MGForwardPos, Normalised);

		if (Angle < -0.7f)
		{
			return true;
		}
	}
	return false;
}

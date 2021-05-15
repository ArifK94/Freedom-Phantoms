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
	MountedGunSightRadius = 500.0f;

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

		OwningCombatCharacter->GetCharacterMovement()->bUseRVOAvoidance = false;

		// Alternative to AI Sight Perception in case 360 sight is wanted
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

		// For detecting MG nearby
		if (MountedGunSphere == nullptr)
		{
			MountedGunSphere = NewObject<USphereComponent>(OwningCombatCharacter);
			if (MountedGunSphere)
			{
				MountedGunSphere->RegisterComponent();
				MountedGunSphere->AttachToComponent(OwningCombatCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				MountedGunSphere->SetSphereRadius(MountedGunSightRadius);
				MountedGunSphere->SetCollisionProfileName(TEXT("OverlapAll"));
			}
		}

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
	if (OwningCombatCharacter->IsInHelicopter())
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
	if (OwningCombatCharacter->IsInHelicopter())
	{
		return EPathFollowingRequestResult::Failed;
	}

	EPathFollowingRequestResult::Type Movment = MoveToLocation(TargetDestination, AcceptRadius, StopOnOverlap, UsePathfinding, ProjectDestinationToNavigation, CanStrafe, FilterClass, AllowPartialPaths);

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
		if (Movment != EPathFollowingRequestResult::AlreadyAtGoal)
		{
			OwningCombatCharacter->BeginSprint();
		}
		else
		{
			OwningCombatCharacter->EndSprint();
		}
	}


	return Movment;
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
		// Attach Follow Camera to head socket
		OwningCombatCharacter->FollowCamera->AttachToComponent(OwningCombatCharacter->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, OwningCombatCharacter->GetHeadSocket());

		GetWorldTimerManager().SetTimer(THandler_BeginFire, this, &ACombatAIController::ShootAtEnemy, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_EndFire, this, &ACombatAIController::EndFiring, FMath::RandRange(TimeBetweenShotsMin, TimeBetweenShotsMax), true);
		GetWorldTimerManager().SetTimer(THandler_MountedGun, this, &ACombatAIController::FindMountedGun, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_FindEnemy, this, &ACombatAIController::FindEnemy, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_CommanderOrders, this, &ACombatAIController::CheckCommanderOrder, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_Sprint, this, &ACombatAIController::UpdateSprint, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_CombatAlert, this, &ACombatAIController::UpdatCombatAlert, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_FindCover, this, &ACombatAIController::FindCover, 1.0f, true);
		//GetWorldTimerManager().SetTimer(THandler_ResetMovement, this, &ACombatAIController::ResetLocation, 2.0f, true);
	}
}

void ACombatAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OwningCombatCharacter)
	{
		UpdateCharacterMovement();

		if (PerceptionComp != nullptr)
		{
			SetVisionAngle();
		}
	}
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

void ACombatAIController::FindEnemy()
{
	AActor* CurrentActor = nullptr;
	TArray<AActor*> ActorsInSight;
	float TargetSightDistance = TargetSightRadius;

	if (TargetSightSphere)
	{
		TargetSightSphere->GetOverlappingActors(ActorsInSight, ABaseCharacter::StaticClass());
	}
	else if (OwningCombatCharacter->IsInHelicopter())
	{
		PerceptionComp->GetKnownPerceivedActors(TSubclassOf<UAISense_Sight>(), ActorsInSight);
	}
	else
	{
		return;
	}

	FVector OwnerLocation = OwningCombatCharacter->GetActorLocation();

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
							CurrentActor = PotentialEnemy;
						}
					}
				}
			}
		}
	}

	if (CurrentActor)
	{
		OwningCombatCharacter->TargetFound();

		if (OwningCombatCharacter->IsUsingMountedWeapon() && MountedGun)
		{
			FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(OwnerLocation, CurrentActor->GetActorLocation());


			bool Check1 = TargetRot.Yaw < MountedGun->GetYawMin();
			bool Check2 = TargetRot.Yaw > MountedGun->GetYawMax();
			bool Check3 = TargetRot.Pitch < MountedGun->GetPitchMin();
			bool Check4 = TargetRot.Pitch > MountedGun->GetPitchMax();
			bool Check5 = IsEnemyBehindMG(CurrentActor);

			// check if enemy position is within turret's pitch and yaw boundaries and is not behind the MG
			if (Check5)
			{
				OwningCombatCharacter->DropMountedGun();
			}
		}
	}
	else
	{
		if (OwningCombatCharacter->IsUsingMountedWeapon() && MountedGun)
		{
			MountedGun->ResetCamera();
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

		if (OwningCombatCharacter->IsUsingMountedWeapon())
		{
			FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(OwningCombatCharacter->GetActorLocation(), EnemyActor->GetActorLocation());
			MountedGun->SetRotatioInput(TargetRot);
		}

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
			PumpActionWeapon = Cast<APumpActionWeapon>(OwningCombatCharacter->GetCurrentWeapon());

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
	else
	{
		OwningCombatCharacter->EndFire();
		OwningCombatCharacter->EndAim();

		ClearFocus(EAIFocusPriority::Gameplay);

		// look straight ahead with the MG direction
		if (OwningCombatCharacter->IsUsingMountedWeapon())
		{
			OwningCombatCharacter->GetCapsuleComponent()->SetWorldRotation(MountedGun->GetCharacterStandRot());
		}


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

	if (PumpActionWeapon) {
		return;
	}

	if (OwningCombatCharacter->IsFiring())
	{
		OwningCombatCharacter->EndFire();
	}
}

void ACombatAIController::FindMountedGun()
{
	// if already using an MG
	if (OwningCombatCharacter->IsUsingMountedWeapon()) {

		// is enemy beind the mounted gun? then drop the MG
		if (IsEnemyBehindMG() || CurrentMovement != EPathFollowingRequestResult::AlreadyAtGoal)
		{
			OwningCombatCharacter->DropMountedGun();
		}
		return;
	}

	if (MountedGun != nullptr) {

		if (CurrentMovement == EPathFollowingRequestResult::AlreadyAtGoal && !IsEnemyBehindMG() && !OwningCombatCharacter->IsReloading())
		{
			OwningCombatCharacter->UseMountedGun(MountedGun);
			return;
		}

		// in case player or another NPC has reached the MG before AI
		if (MountedGun->GetPotentialOwner() != OwningCombatCharacter || MountedGun->GetOwner() != nullptr)
		{
			MountedGun = nullptr;
			CanFindCover = true;
		}
		else
		{
			CurrentMovement = MoveToTarget(0.0f, false);
			CanFindCover = false;
			return;
		}
	}

	AMountedGun* SelectedMG = nullptr;
	TArray<AActor*> MountedGunsRadius;
	float TargetSightDistance = TargetSightRadius;

	FVector CharacterLocation = OwningCombatCharacter->GetActorLocation();

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

				if (HasNoOwner || IsSamePotentialOwner)
				{
					FVector MGLocation = PotentialMG->GetActorLocation();

					float DistanceDiff = FVector::Dist(CharacterLocation, MGLocation);

					if (DistanceDiff < TargetSightDistance)
					{
						TargetSightDistance = DistanceDiff;
						SelectedMG = PotentialMG;
					}
				}
			}
		}
	}
	else
	{
		if (MountedGunSphere == nullptr) {
			return;
		}

		MountedGunSphere->GetOverlappingActors(MountedGunsRadius, AMountedGun::StaticClass());

		for (int index = 0; index < MountedGunsRadius.Num(); index++)
		{

			AMountedGun* PotentialMG = Cast<AMountedGun>(MountedGunsRadius[index]);

			// check if mounted gun is present in the stronghold and has no owner as well as no potential owner in case another AI wishes to use it
			if (PotentialMG && PotentialMG->GetPotentialOwner() == nullptr && PotentialMG->GetOwner() == nullptr)
			{

				FVector MGLocation = PotentialMG->GetActorLocation();

				float DistanceDiff = FVector::Dist(CharacterLocation, MGLocation);

				if (DistanceDiff < TargetSightDistance)
				{
					TargetSightDistance = DistanceDiff;
					SelectedMG = PotentialMG;
				}
			}
		}
	}

	MountedGun = SelectedMG;

	if (MountedGun)
	{
		MountedGun->SetPotentialOwner(OwningCombatCharacter);
		TargetDestination = MountedGun->GetCharacterStandPos();
		CanFindCover = false;
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
	if (OwningCombatCharacter->IsInHelicopter()) {
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
	if (MountedGun == nullptr) {
		return false;
	}

	if (Enemy == nullptr) {
		Enemy = EnemyActor;
	}

	if (Enemy)
	{
		FVector MGForwardPos = UKismetMathLibrary::GetForwardVector(MountedGun->GetActorRotation());
		FVector Normalised = Enemy->GetActorLocation() - MountedGun->GetActorLocation();
		UKismetMathLibrary::Vector_Normalize(Normalised);
		float Angle = UKismetMathLibrary::Dot_VectorVector(MGForwardPos, Normalised);

		if (Angle < -0.7f)
		{
			return true;
		}
	}
	return false;
}

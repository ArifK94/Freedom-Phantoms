#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"
#include "CustomComponents/AIMovementComponent.h"
#include "CustomComponents/PatrolFollowerComponent.h"
#include "CustomComponents/CoverFinderComponent.h"
#include "CustomComponents/AI/StrongholdDefenderComponent.h"
#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/MountedGunFinderComponent.h"

#include "Services/SharedService.h"

#include "AI/UtilityAIComponent.h"
#include "AI/Actions/CombatAction.h"
#include "AI/Actions/CoverAction.h"
#include "AI/Actions/MountedGunAction.h"
#include "AI/Actions/RecruitFollowAction.h"
#include "AI/Actions/RecruitDefendAction.h"
#include "AI/Actions/RecruitAttackAction.h"
#include "AI/Actions/StrongholdAction.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/EngineTypes.h"
#include "NavigationSystem.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ACombatAIController::ACombatAIController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	AcceptanceRadius = 300.0f;

	TimeBetweenShotsMin = 2.0f;
	TimeBetweenShotsMax = 3.0f;

	DestinationRadius = 300.0f;

	GrenadeThrowTimeMin = 8.f;
	GrenadeThrowTimeMax = 15.f;

	MoveToLastSeenEnemy = true;

	AIMovementComponent = CreateDefaultSubobject<UAIMovementComponent>(TEXT("AIMovementComponent"));
	CoverFinderComponent = CreateDefaultSubobject<UCoverFinderComponent>(TEXT("CoverFinderComponent"));

	UtilityAIComponent = CreateDefaultSubobject<UUtilityAIComponent>(TEXT("UtilityAIComponent"));
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

	if (!GetPawn()) {
		return;
	}

	m_DelaTime = DeltaTime;

	if (!OwningCombatCharacter->GetHealthComp()->IsAlive()) {
		return;
	}

	if (EnemyActor)
	{
		// if using a mounted gun
		if (OwningCombatCharacter->IsUsingMountedWeapon() && OwningCombatCharacter->GetMountedGun())
		{
			MountedGunFinderComponent->FocusTarget(OwningCombatCharacter->GetMountedGun(), EnemyActor->GetActorLocation());
		}
		else
		{
			SetFocus(EnemyActor);
			//SetFocalPoint(TargetSearchParams->TargetLocation);
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

	if (!StrongholdDefenderComponent)
	{
		StrongholdDefenderComponent = NewObject<UStrongholdDefenderComponent>(OwningCombatCharacter);

		if (StrongholdDefenderComponent)
		{
			StrongholdDefenderComponent->RegisterComponent();
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

	// if assigned an MG at the beginning
	if (OwningCombatCharacter->GetMountedGun()) {
		TargetDestination = OwningCombatCharacter->GetMountedGun()->GetActorLocation();
	}


	if (UtilityAIComponent) {
		UtilityAIComponent->SpawnActionInstance(UCombatAction::StaticClass());
		//UtilityAIComponent->SpawnActionInstance(UCoverAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(UMountedGunAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(URecruitFollowAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(URecruitDefendAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(URecruitAttackAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(UStrongholdAction::StaticClass());
	}

}

void ACombatAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (UtilityAIComponent) {
		UtilityAIComponent->EnableUtilityAI = true;
	}

	// get owning character
	OwningCombatCharacter = Cast<ACombatCharacter>(InPawn);

	Init();

	CanFindCover = true;

	if (OwningCombatCharacter)
	{
		// Attach Follow Camera to head socket
		OwningCombatCharacter->SetFirstPersonView();

		GetWorldTimerManager().SetTimer(THandler_PatrolStart, this, &ACombatAIController::StartPatrol, 1.0f, true);
		GetWorldTimerManager().SetTimer(THandler_CommanderOrders, this, &ACombatAIController::CheckCommanderOrder, 1.0f, true);


		// Events! DON'T FORGET TO REMOVE EVENTS ON THE UNPOSSESS() method to prevent a crash when switching between possess & unpossess

		if (TargetFinderComponent && !TargetFinderComponent->OnTargetSearch.IsBound()) {
			TargetFinderComponent->OnTargetSearch.AddDynamic(this, &ACombatAIController::OnTargetSearchUpdate);
		}

		if (!OwningCombatCharacter->OnRappelUpdate.IsBound()) {
			OwningCombatCharacter->OnRappelUpdate.AddDynamic(this, &ACombatAIController::OnRappelUpdated);
		}


		if (AIMovementComponent) {
			if (!AIMovementComponent->OnDestinationSet.IsBound()) {
				AIMovementComponent->OnDestinationSet.AddDynamic(this, &ACombatAIController::OnMovementDestinationSet);
			}

			if (!AIMovementComponent->OnDestinationReached.IsBound()) {
				AIMovementComponent->OnDestinationReached.AddDynamic(this, &ACombatAIController::OnMovementDestinationReached);
			}
		}

		if (TargetFinderComponent) {
			TargetFinderComponent->SetFindTargetPerFrame(true);
		}

		if (StrongholdDefenderComponent) {
			if (!StrongholdDefenderComponent->OnDefenderPointFound.IsBound()) {
				StrongholdDefenderComponent->OnDefenderPointFound.AddDynamic(this, &ACombatAIController::OnStrongholdPointFound);
			}
		}

		UHealthComponent* HealthComp = OwningCombatCharacter->GetHealthComp();

		if (HealthComp) {
			if (!HealthComp->OnHealthChanged.IsBound()) {
				HealthComp->OnHealthChanged.AddDynamic(this, &ACombatAIController::OnHealthUpdate);
			}
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

	ClearTimers();

	if (OwningCombatCharacter)
	{
		if (TargetFinderComponent && TargetFinderComponent->OnTargetSearch.IsBound()) {
			TargetFinderComponent->OnTargetSearch.RemoveDynamic(this, &ACombatAIController::OnTargetSearchUpdate);
		}

		if (OwningCombatCharacter->OnRappelUpdate.IsBound()) {
			OwningCombatCharacter->OnRappelUpdate.RemoveDynamic(this, &ACombatAIController::OnRappelUpdated);
		}


		if (AIMovementComponent)
		{
			if (AIMovementComponent->OnDestinationSet.IsBound()) {
				AIMovementComponent->OnDestinationSet.RemoveDynamic(this, &ACombatAIController::OnMovementDestinationSet);
			}

			if (AIMovementComponent->OnDestinationReached.IsBound()) {
				AIMovementComponent->OnDestinationReached.RemoveDynamic(this, &ACombatAIController::OnMovementDestinationReached);
			}
		}


		if (StrongholdDefenderComponent) {
			if (StrongholdDefenderComponent->OnDefenderPointFound.IsBound()) {
				StrongholdDefenderComponent->OnDefenderPointFound.RemoveDynamic(this, &ACombatAIController::OnStrongholdPointFound);
			}
		}


		if (Commander) {
			Commander->OnOrderSent.RemoveDynamic(this, &ACombatAIController::OnOrderReceived);
			Commander = nullptr;
		}

		UHealthComponent* HealthComp = OwningCombatCharacter->GetHealthComp();

		if (HealthComp) {
			if (HealthComp->OnHealthChanged.IsBound()) {
				HealthComp->OnHealthChanged.RemoveDynamic(this, &ACombatAIController::OnHealthUpdate);
			}
		}
	}
}

void ACombatAIController::ClearTimers()
{
	if (TargetFinderComponent) {
		TargetFinderComponent->SetFindTargetPerFrame(false);
	}

	if (UtilityAIComponent) {
		UtilityAIComponent->EnableUtilityAI = false;
	}

	GetWorldTimerManager().ClearTimer(THandler_CommanderOrders);
	GetWorldTimerManager().ClearTimer(THandler_FindCover);
	GetWorldTimerManager().ClearTimer(THandler_PatrolStart);
}

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
}

void ACombatAIController::OnOrderReceived(UCommanderRecruit* RecruitInfo)
{
	// ensure the owning character received the order
	if (RecruitInfo->Recruit != OwningCombatCharacter) {
		return;
	}

	bRecruitInfo = RecruitInfo;

	CurrentCommand = RecruitInfo->CurrentCommand;
	TargetDestination = RecruitInfo->TargetLocation;

	if (OwningCombatCharacter->IsTakingCover())
	{
		OwningCombatCharacter->StopCover();
	}

	OwningCombatCharacter->DropMountedGun();


	OwningCombatCharacter->GetCharacterMovement()->bUseRVOAvoidance = true;

	CanFindCover = true;
	HasChosenNearTargetDest = false;


	StayCombatAlert = false;
	UpdatCombatAlert();
}

void ACombatAIController::OnStrongholdPointFound(FStrongholdDefenderParams StrongholdDefenderParams)
{
	//OwningCombatCharacter->GetHealthComp()->SetCanBeWounded(false);

	//TargetDestination = StrongholdDefenderParams.TargetPoint;
	//AIMovementComponent->MoveToDestination(TargetDestination, .0f, AIBehaviourState::PriorityDestination);
}

void ACombatAIController::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	if (!InHealthParameters.AffectedHealthComponent->IsAlive())
	{
		ClearTimers();

		if (StrongholdDefenderComponent->GetStronghold()) {
			OwningCombatCharacter->GetHealthComp()->SetCanBeWounded(true);
			StrongholdDefenderComponent->RemoveStronghold();
		}

		// destroy the controller if not wounded
		if (!InHealthParameters.AffectedHealthComponent->GetIsWounded()) {
			OwningCombatCharacter->DetachFromControllerPendingDestroy();
		}
	}
}

void ACombatAIController::OnRappelUpdated(ABaseCharacter* BaseCharacter)
{
	// Find a random point when landed after rappellinh so upcoming characters rapelling down do not stand in the same spot
	if (!OwningCombatCharacter->GetIsExitingVehicle())
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

	EnemyActor = ChosenTarget;
}

/**
* Prioritise Actors to flee from.
*/
void ACombatAIController::OnNearbyActorFound_Implementation(FAvoidableParams AvoidableParams)
{
	FVector OwnerLocation = OwningCombatCharacter->GetActorLocation();
	FVector AvoidableLocation = AvoidableParams.Actor->GetActorLocation();

	// Distance away from the avoidable actor.
	FVector DirectionAvoidance = UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::FindLookAtRotation(OwnerLocation, AvoidableLocation));
	DirectionAvoidance = (DirectionAvoidance * (AvoidableParams.AvoidableDistance * -1.f)) + OwnerLocation;


	// get a random reachable point away from avoidance distance to make the move to dynamic
	FNavLocation NavLocation;
	UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->GetRandomReachablePointInRadius(DirectionAvoidance, AvoidableParams.AvoidableDistance, NavLocation);


	// move to the point away from the avoidable
	TargetDestination = NavLocation.Location;
	AIMovementComponent->MoveToDestination(TargetDestination, 20.f, AIBehaviourState::PriorityDestination);

	if (OwningCombatCharacter->GetVoiceAudioComponent()->Sound != OwningCombatCharacter->GetVoiceClipsSet()->GrenadeIncomingSound || !OwningCombatCharacter->GetVoiceAudioComponent()->IsPlaying()) {
		OwningCombatCharacter->PlayVoiceSound(OwningCombatCharacter->GetVoiceClipsSet()->GrenadeIncomingSound);
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


void ACombatAIController::UpdateLastSeen()
{
	// look straight ahead with the MG direction
	if (OwningCombatCharacter->IsUsingMountedWeapon() && OwningCombatCharacter->GetMountedGun() && OwningCombatCharacter->GetMountedGun()->GetAdjustBehindMG())
	{
		OwningCombatCharacter->GetCapsuleComponent()->SetWorldRotation(OwningCombatCharacter->GetMountedGun()->GetCharacterStandRot());
	}

	if (OwningCombatCharacter->GetCurrentWeapon()->GetCurrentAmmo() <= 0 || OwningCombatCharacter->GetCurrentWeapon()->GetCurrentAmmo() < OwningCombatCharacter->GetCurrentWeapon()->getAmmoPerClip()) // reload clip if finished completely or  reload if not on full clip
	{
		OwningCombatCharacter->BeginReload();
	}
	else
	{
		// switch back to primary
		if (OwningCombatCharacter->CanSwapWeapon() && OwningCombatCharacter->GetCurrentWeapon() == OwningCombatCharacter->GetSecondaryWeaponObj())
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

void ACombatAIController::MoveToRandomPoint()
{
	// if following a commander or defending a stronghold, then do not move to random point
	if (Commander && CurrentCommand == CommanderOrders::Follow || StrongholdDefenderComponent->GetStronghold() != nullptr) {
		return;
	}


	auto Movement = AIMovementComponent->MoveToDestination(TargetDestination, .0f, AIBehaviourState::Normal);
	//auto Movement = MoveToTarget(0.0f);

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
	Commander = OwningCombatCharacter->GetCommander();

	if (Commander == nullptr) {
		return;
	}

	OwningCombatCharacter->DropMountedGun();


	// Assign Order Event
	Commander->OnOrderSent.AddDynamic(this, &ACombatAIController::OnOrderReceived);
	StayCombatAlert = false; // refresh state of behaviour

	// if NPC was a stronghold defender, then rmeove the stronghold memory actor & assign the wounded flag to true as it is a now a recruit of the commander.
	if (StrongholdDefenderComponent->GetStronghold()) {
		OwningCombatCharacter->GetHealthComp()->SetCanBeWounded(true);
		StrongholdDefenderComponent->RemoveStronghold();
	}

	SetBehaviourState(AIBehaviourState::PriorityOrdersCommander);

	GetWorldTimerManager().ClearTimer(THandler_CommanderOrders);
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
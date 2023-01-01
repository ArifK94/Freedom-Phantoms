#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"
#include "Vehicles/VehicleBase.h"
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
#include "AI/Actions/AvoidanceAction.h"
#include "AI/Actions/PatrolAction.h"
#include "AI/Actions/LastSeenEnemyAction.h"
#include "AI/Actions/PriorityDestinationAction.h"

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
	UtilityAIComponent = CreateDefaultSubobject<UUtilityAIComponent>(TEXT("UtilityAIComponent"));
	AIMovementComponent = CreateDefaultSubobject<UAIMovementComponent>(TEXT("AIMovementComponent"));

	PrimaryActorTick.bCanEverTick = true;

	AcceptanceRadius = 300.0f;

	TimeBetweenShotsMin = 2.0f;
	TimeBetweenShotsMax = 3.0f;

	DestinationRadius = 300.0f;

	GrenadeThrowTimeMin = 8.f;
	GrenadeThrowTimeMax = 15.f;

	MoveToLastSeenEnemy = true;

	NonBlindFireWeaponTypes.Add(WeaponType::Shotgun);
	NonBlindFireWeaponTypes.Add(WeaponType::RPG);
}


void ACombatAIController::BeginPlay()
{
	Super::BeginPlay();

	DefaultDestinationRadius = DestinationRadius;

	EnemyCloseRange = FMath::RandRange(500.0f, 1000.0f);
	TimeSpentOnEnemyRange = FMath::RandRange(10.f, 20.0f);
}

void ACombatAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bDeltaTime = DeltaTime;


	if (!GetPawn() || !OwningCombatCharacter) {
		return;
	}


	if (!OwningCombatCharacter->GetHealthComp()->IsAlive()) {
		return;
	}

	if (!OwningCombatCharacter->GetMountedGun())
	{
		FindMountedGun();
	}

	// AI can take out vehicles if holds an RPG
	if (OwningCombatCharacter && TargetFinderComponent)
	{
		if ((OwningCombatCharacter->GetPrimaryWeapon() && OwningCombatCharacter->GetPrimaryWeapon()->GetWeaponType() == WeaponType::RPG) || (OwningCombatCharacter->GetSecondaryWeaponObj() && OwningCombatCharacter->GetSecondaryWeaponObj()->GetWeaponType() == WeaponType::RPG))
		{
			if (!TargetFinderComponent->DoesClassFilterExist(AVehicleBase::StaticClass()))
			{
				TargetFinderComponent->AddClassFilter(AVehicleBase::StaticClass());
			}
		}
	}

	OwningCombatCharacter->bUseControllerRotationYaw = false;
	OwningCombatCharacter->GetCharacterMovement()->RotationRate = FRotator(0.0f, 250.f, 0.0f); // ...at this rotation rate
}

void ACombatAIController::Init()
{
	if (OwningCombatCharacter == nullptr) {
		return;
	}

	DisableCombat = false;
	//OwningCombatCharacter->GetCharacterMovement()->bUseRVOAvoidance = true;
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
		MountedGunFinderComponent = NewObject<UMountedGunFinderComponent>(this);

		if (MountedGunFinderComponent)
		{
			MountedGunFinderComponent->RegisterComponent();
		}
	}

	// if assigned an MG at the beginning
	if (OwningCombatCharacter->GetMountedGun()) {
		TargetDestination = OwningCombatCharacter->GetMountedGun()->GetActorLocation();
	}


	if (UtilityAIComponent) 
	{
		UtilityAIComponent->SpawnActionInstance(UCombatAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(UCoverAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(UMountedGunAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(URecruitFollowAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(URecruitDefendAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(URecruitAttackAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(UStrongholdAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(UAvoidanceAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(UPatrolAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(ULastSeenEnemyAction::StaticClass());
		UtilityAIComponent->SpawnActionInstance(UPriorityDestinationAction::StaticClass());
	}

}

void ACombatAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (UtilityAIComponent) {
		UtilityAIComponent->SetEnableUtilityAI(true);
	}

	// run behavior tree if specified
	if (BTAsset)
	{
		AAIController::RunBehaviorTree(BTAsset);
	}

	// get owning character
	OwningCombatCharacter = Cast<ACombatCharacter>(InPawn);

	Init();

	if (OwningCombatCharacter)
	{
		// Attach Follow Camera to head socket
		OwningCombatCharacter->SetFirstPersonView();

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

		UHealthComponent* HealthComp = OwningCombatCharacter->GetHealthComp();

		if (HealthComp) {
			if (!HealthComp->OnHealthChanged.IsBound()) {
				HealthComp->OnHealthChanged.AddDynamic(this, &ACombatAIController::OnHealthUpdate);
			}
		}

		MountedGunFinderComponent->ResetSearchRadius();
		CoverFinderComponent->ResetSearchRadius();
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
		UtilityAIComponent->SetEnableUtilityAI(false);
	}

	GetWorldTimerManager().ClearTimer(THandler_CommanderOrders);
}

void ACombatAIController::OnMovementDestinationSet(AIBehaviourState BehaviourState)
{
	SetBehaviourState(BehaviourState);

	OwningCombatCharacter->StopCover();
}

void ACombatAIController::OnMovementDestinationReached(FVector Destination)
{

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

	if (CurrentCommand == CommanderOrders::Defend)
	{
		// Reduce search radius for all components if defending.
		MountedGunFinderComponent->SetSearchRadius(100.f);
		CoverFinderComponent->SetSearchRadius(200.f);
	}
	else
	{
		MountedGunFinderComponent->ResetSearchRadius();
		CoverFinderComponent->ResetSearchRadius();
	}

	OwningCombatCharacter->DropMountedGun();

	//OwningCombatCharacter->GetCharacterMovement()->bUseRVOAvoidance = true;

	HasChosenNearTargetDest = false;
	IsRunningForCover = false;
	CoverFound = false;

	SetStayCombatAlert(false);
	
	MoveToOrderResult = EPathFollowingRequestResult::Failed;

	CoverFinderComponent->RemoveCoverPoint();
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
			DestinationRadius = 500.f;
			FNavLocation NavLocationTwo;
			bool bOnNavMeshTwo = NavigationArea->GetRandomReachablePointInRadius(NavLocation.Location, DestinationRadius, NavLocationTwo);

			if (bOnNavMeshTwo)
			{
				TargetDestination = NavLocationTwo.Location;
				GetWorldTimerManager().SetTimer(THandler_MoveToNearbyDestination, this, &ACombatAIController::MoveToRandomPoint, 1.f, true);
			}
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

	// if no new enemy found,
	// maintain current enemy if enemy or owning AI character are taking cover.
	if (ChosenTarget == nullptr && EnemyActor) 
	{
		auto EnemyCharacter = Cast<ACombatCharacter>(EnemyActor);
		
		auto IsTargetClose = SharedService::IsNearTargetPosition(OwningCombatCharacter, EnemyActor, EnemyCloseRange);

		bool IsEnemyAlive = UHealthComponent::IsActorAlive(EnemyCharacter);

		// is enemy alive?
		// AI can still see enemy if either enemy.
		// or is my AI taking cover & the enemy is close? Should not be able to 
		if (IsEnemyAlive && (EnemyCharacter && EnemyCharacter->IsTakingCover()) || (OwningCombatCharacter->IsTakingCover() && IsTargetClose))
		{
			TimeSpentOnEnemy++;
			return;
		}
		// otherwise enemy is unreachable.
		else
		{
			// if enemy is still alive, then this is the last seen enemy.
			if (IsEnemyAlive)
			{
				LastSeenEnemyActor = EnemyActor;
				LastSeenLocation = EnemyActor->GetActorLocation();
			}
			EnemyActor = nullptr;
		}
	}
	else 
	{
		// same enemy as before?
		TimeSpentOnEnemy = EnemyActor == ChosenTarget ? TimeSpentOnEnemy + 1 : .0f;

		EnemyActor = ChosenTarget;

		// if enemy is present, then ignore last seen enemy.
		if (EnemyActor) {
			LastSeenEnemyActor = nullptr;
		}
	}
}

/**
* Prioritise Actors to flee from.
*/
void ACombatAIController::OnNearbyActorFound_Implementation(FAvoidableParams AvoidableParams)
{
	bAvoidableParams = AvoidableParams;

	if (OwningCombatCharacter->GetVoiceAudioComponent()->Sound != OwningCombatCharacter->GetVoiceClipsSet()->GrenadeIncomingSound ||
		!OwningCombatCharacter->GetVoiceAudioComponent()->IsPlaying()) {
		OwningCombatCharacter->PlayVoiceSound(OwningCombatCharacter->GetVoiceClipsSet()->GrenadeIncomingSound);
	}
}


void ACombatAIController::MoveToRandomPoint()
{
	// if following a commander or defending a stronghold, then do not move to random point
	if (Commander && CurrentCommand == CommanderOrders::Follow || StrongholdDefenderComponent->GetStronghold() != nullptr) {
		return;
	}


	auto Movement = AIMovementComponent->MoveToDestination(TargetDestination, .0f, AIBehaviourState::Normal);

	// Find a random point around destination if has arrived at the original target destination
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

		SetStayCombatAlert(true);

		GetWorldTimerManager().ClearTimer(THandler_MoveToNearbyDestination);
	}
}

bool ACombatAIController::CanBlindCoverFire(AWeapon* Weapon)
{
	// if the weapon param has the type which cannot alow blind fire while in cover then return false.
	for (auto WeaponType : NonBlindFireWeaponTypes)
	{
		if (Weapon->GetWeaponType() == WeaponType)
		{
			return false;
		}
	}

	return true;
}

void ACombatAIController::FindMountedGun()
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

			if (!IsInRange || SharedService::IsTargetBehind(SelectedMG, EnemyActor))
			{
				IsMGValid = false;
			}
		}

		if (IsMGValid) 
		{
			SelectedMG->SetPotentialOwner(OwningCombatCharacter);
			OwningCombatCharacter->SetMountedGun(SelectedMG);
		}
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
	SetStayCombatAlert(false); // refresh state of behaviour

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

void ACombatAIController::SetStayCombatAlert(bool Alert)
{
	StayCombatAlert = Alert;

	UpdatCombatAlert();
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

void ACombatAIController::SetFocalPosition(FVector TargetLocation)
{
	FRotator NewControlRotation = GetControlRotation();

	// Look toward focus
	const FVector FocalPoint = TargetLocation;
	NewControlRotation = (FocalPoint - OwningCombatCharacter->GetPawnViewLocation()).Rotation();

	SetControlRotation(NewControlRotation);

	const FRotator CurrentPawnRotation = OwningCombatCharacter->GetActorRotation();

	if (CurrentPawnRotation.Equals(NewControlRotation, 1e-3f) == false)
	{
		OwningCombatCharacter->FaceRotation(NewControlRotation, bDeltaTime);
	}
}


void ACombatAIController::SetPriorityDestination(FVector Location)
{
	// ignore if location is zero.
	if (Location.IsZero()) {
		return;
	}

	PriorityLocation = Location;
	HasPriorityDestination = true;
}
#include "Objectives/Stronghold.h"
#include "Characters/CombatCharacter.h"
#include "Controllers/CombatAIController.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/CoverPointComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "CustomComponents/AI/StrongholdDefenderComponent.h"

#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"


AStronghold::AStronghold()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	StrongholdArea = CreateDefaultSubobject<UBoxComponent>(TEXT("StrongholdArea"));
	StrongholdArea->SetCollisionProfileName(TEXT("OverlapAll"));
	StrongholdArea->CanCharacterStepUpOn = ECB_No;
	StrongholdArea->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	SpawnBox->SetCollisionProfileName(TEXT("NoCollision"));
	SpawnBox->CanCharacterStepUpOn = ECB_No;
	SpawnBox->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);

	FactionFlag = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FactionFlag"));
	FactionFlag->SetCollisionProfileName(TEXT("NoCollision"));
	FactionFlag->CanCharacterStepUpOn = ECB_No;
	FactionFlag->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	FlagClothMaterialIndex = 0;

	StrongholdAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("StrongholdAudio"));
	StrongholdAudio->SetupAttachment(RootComponent);

	SpawnMax = 5;
	SpawnRate = 1.f;

	SpawnAreaComponentTag = "SpawnArea";
}

void AStronghold::BeginPlay()
{
	Super::BeginPlay();

	DominantFaction = new FOccupiedFaction();
	DominantFaction->Faction = TeamFaction::Neutral;
	DominantFaction->FlagMaterial = NeutralFlagMaterial;
	DominantFaction->FactionCount = 0;
	DominantFaction->FactionDataSet = nullptr;

	GetSpawnAreas();
	LoadCoverPoints();

	GetWorldTimerManager().SetTimer(THandler_SpawnDelay, this, &AStronghold::SpawnDefender, SpawnRate, true);
	GetWorldTimerManager().SetTimer(THandler_OverlappingCombatatant, this, &AStronghold::CheckOverlappingCombatatant, 0.5f, true);
}

void AStronghold::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTotalOccupants();

	GetHighestFaction();
}

void AStronghold::CheckOverlappingCombatatant()
{
	TArray<AActor*> OverlappingActors;
	StrongholdArea->GetOverlappingActors(OverlappingActors, ACombatCharacter::StaticClass());


	for (int i = 0; i < OverlappingActors.Num(); i++)
	{
		auto CombatCharacter = Cast<ACombatCharacter>(OverlappingActors[i]);

		if (!CombatCharacter) {
			continue;
		}

		if (!UHealthComponent::IsAlive(CombatCharacter)) {
			continue;
		}

		// avoid if in a vehicle or other non walking activities. Stronghold should only be affected by characters on foot.
		if (CombatCharacter->GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Walking) {
			return;
		}

		if (!DoesOccupantExist(CombatCharacter)) {
			TotalOccupyingCombatants.Add(CombatCharacter);
		}

	}
}

void AStronghold::GetSpawnAreas()
{
	SpawnAreas.Add(SpawnBox);

	TArray<UActorComponent*> SpawnPoints = GetComponentsByTag(UBoxComponent::StaticClass(), SpawnAreaComponentTag);

	for (int i = 0; i < SpawnPoints.Num(); i++)
	{
		UBoxComponent* SpawnArea = Cast<UBoxComponent>(SpawnPoints[i]);
		if (SpawnArea)
		{
			SpawnAreas.Add(SpawnArea);
		}
	}
}

void AStronghold::LoadCoverPoints()
{
	TArray<UCoverPointComponent*> AllCoverPoints;
	GetComponents<UCoverPointComponent>(AllCoverPoints);

	for (int i = 0; i < AllCoverPoints.Num(); i++)
	{
		auto CoverPoint = AllCoverPoints[i];

		if (CoverPoint && !CoverPoints.Contains(CoverPoint)) {
			CoverPoints.Add(CoverPoint);
		}
	}
}

UCoverPointComponent* AStronghold::GetCoverPoint(AActor* OwningCharacter)
{
	// look for any free priority points first.
	for (int i = 0; i < CoverPoints.Num(); i++)
	{
		if (!CoverPoints[i]->GetOwner() && CoverPoints[i]->GetIsPriority()) {
			CoverPoints[i]->SetOccupant(OwningCharacter);
			return CoverPoints[i];
		}
	}

	// if no priority points found then choose any free point.
	for (int i = 0; i < CoverPoints.Num(); i++)
	{
		if (!CoverPoints[i]->GetOwner()) {
			CoverPoints[i]->SetOccupant(OwningCharacter);
			return CoverPoints[i];
		}
	}
	return nullptr;
}

void AStronghold::SpawnDefender()
{
	// check if captured by a faction
	if (DominantFaction->Faction == TeamFaction::Neutral) {
		return;
	}

	if (DefendingCombatatants.Num() >= SpawnMax) {
		UpdateDefenders();
		return;
	}

	if (DominantFaction->FactionDataSet == nullptr) {
		return;
	}

	if (DominantFaction->FactionDataSet->OperativeCharacterClass == nullptr) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int RandIndex = rand() % SpawnAreas.Num();

	UBoxComponent* SpawnArea = SpawnAreas[RandIndex];

	FVector Location = UKismetMathLibrary::RandomPointInBoundingBox(SpawnArea->GetComponentLocation(), SpawnArea->GetScaledBoxExtent());

	// check if location is on the navmesh
	FNavLocation NavLocation;
	UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	bool navResult = NavigationArea->GetRandomReachablePointInRadius(Location, 500.f, NavLocation);

	if (!navResult) {
		return;
	}

	FNavLocation NewNavLocation;
	navResult = NavigationArea->ProjectPointToNavigation(NavLocation.Location, NewNavLocation);

	if (!navResult) {
		return;
	}

	auto Character = GetWorld()->SpawnActor<ABaseCharacter>(DominantFaction->FactionDataSet->OperativeCharacterClass, NewNavLocation.Location, SpawnArea->GetComponentRotation(), SpawnParams);

	if (!Character) {
		return;
	}

	UStrongholdDefenderComponent::SetDefender(Character, this);

	auto CombatCharacter = Cast<ACombatCharacter>(Character);
	DefendingCombatatants.Add(CombatCharacter);

	auto Defender = FStrongholdDefender();
	Defender.CombatCharacter = CombatCharacter;
	OnStrongholdDefenderSpawned.Broadcast(Defender);
}

void AStronghold::UpdateTotalOccupants()
{
	if (TotalOccupyingCombatants.Num() <= 0) {
		return;
	}

	for (int i = TotalOccupyingCombatants.Num() - 1; i >= 0; i--)
	{
		ACombatCharacter* Character = TotalOccupyingCombatants[i];

		if (Character)
		{
			auto TeamFactionComp = Cast<UTeamFactionComponent>(Character->GetComponentByClass(UTeamFactionComponent::StaticClass()));

			if (TeamFactionComp)
			{
				FOccupiedFaction* OccupiedFaction = DoesFactionExist(TeamFactionComp->GetSelectedFaction());
				if (OccupiedFaction != nullptr)
				{
					OccupiedFaction->FactionCount++;
				}
				else
				{
					AddFaction(Character, TeamFactionComp->GetSelectedFaction());
				}
			}
			else
			{
				TotalOccupyingCombatants.RemoveAt(i);
			}
		}
		else
		{
			TotalOccupyingCombatants.RemoveAt(i);
		}
	}
}

void AStronghold::UpdateDefenders()
{
	if (DefendingCombatatants.Num() <= 0) {
		return;
	}

	for (int i = DefendingCombatatants.Num() - 1; i >= 0; i--)
	{
		auto Character = DefendingCombatatants[i];

		// if dead or has been recruited by commander then it is no longer a stronghold 
		if (!UHealthComponent::IsAlive(Character) || Character->GetCommander() != nullptr)
		{
			RemoveDefender(Character);
		}
	}
}

void AStronghold::AddFaction(ACombatCharacter* Character, TeamFaction Faction)
{
	FOccupiedFaction* OccupyingFaction = new FOccupiedFaction;
	OccupyingFaction->Faction = Faction;
	OccupyingFaction->FactionCount = 1;
	OccupyingFaction->FactionDataSet = Character->GetFactionDataSet();
	OccupyingFaction->FlagMaterial = OccupyingFaction->FactionDataSet->FlagMaterial;

	OccupiedFactions.Add(OccupyingFaction);
}

FOccupiedFaction* AStronghold::DoesFactionExist(TeamFaction Faction)
{
	if (OccupiedFactions.Num() <= 0) {
		return nullptr;
	}

	for (FOccupiedFaction* OccupiedFaction : OccupiedFactions)
	{
		if (OccupiedFaction->Faction == Faction)
		{
			return OccupiedFaction;
		}
	}

	return nullptr;
}

bool AStronghold::DoesOccupantExist(ACombatCharacter* Occupant)
{
	if (TotalOccupyingCombatants.Num() <= 0) {
		return false;
	}

	for (int i = 0; i < TotalOccupyingCombatants.Num(); i++)
	{
		ACombatCharacter* Character = TotalOccupyingCombatants[i];

		if (Character == Occupant)
		{
			return true;
		}
	}

	return false;
}

void AStronghold::GetHighestFaction()
{
	if (OccupiedFactions.Num() < 0) {
		return;
	}

	FOccupiedFaction* OwningFaction = new FOccupiedFaction;
	OwningFaction->Faction = TeamFaction::Neutral;
	OwningFaction->FlagMaterial = NeutralFlagMaterial;
	OwningFaction->FactionCount = 0;

	for (FOccupiedFaction* OccupiedFaction : OccupiedFactions)
	{
		if (OccupiedFaction->FactionCount > OwningFaction->FactionCount)
		{
			OwningFaction->FactionCount = OccupiedFaction->FactionCount;
			OwningFaction->Faction = OccupiedFaction->Faction;
			OwningFaction->FlagMaterial = OccupiedFaction->FlagMaterial;
			OwningFaction->FactionDataSet = OccupiedFaction->FactionDataSet;
		}
	}

	FOccupiedFaction OccupiedFaction = FOccupiedFaction();
	OccupiedFaction.FactionCount = DominantFaction->FactionCount;
	OccupiedFaction.Faction = DominantFaction->Faction;
	OccupiedFaction.FlagMaterial = DominantFaction->FlagMaterial;
	OccupiedFaction.FactionDataSet = DominantFaction->FactionDataSet;

	// play the capture sound
	if (DominantFaction->Faction == TeamFaction::Neutral) {

		if (CaptureSound != NULL)
		{
			StrongholdAudio->Sound = CaptureSound;
			StrongholdAudio->Play();
		}

		OnStrongholdCaptured.Broadcast(OccupiedFaction);
	}
	else
	{
		// Stronghold has been overrun if the dominant faction is no longer the same
		if (DominantFaction->Faction != OwningFaction->Faction)
		{
			if (OverrunSound != NULL)
			{
				StrongholdAudio->Sound = OverrunSound;
				StrongholdAudio->Play();
			}

			OnStrongholdCaptured.Broadcast(OccupiedFaction);
		}
	}

	// update the dominant faction
	DominantFaction = OwningFaction;
	FactionFlag->SetMaterial(FlagClothMaterialIndex, DominantFaction->FlagMaterial);
}

void AStronghold::RemoveDefender(AActor* Actor)
{
	// remove actor from defender list
	for (int i = 0; i < DefendingCombatatants.Num(); i++)
	{
		if (DefendingCombatatants[i] == Actor) {
			DefendingCombatatants.RemoveAt(i);
			break;
		}
	}

	// reset cover point if actor has been assigned to one
	for (int i = 0; i < CoverPoints.Num(); i++)
	{
		if (CoverPoints[i]->GetOwner() == Actor) {
			CoverPoints[i]->SetOccupant(nullptr);
			break;
		}
	}
}
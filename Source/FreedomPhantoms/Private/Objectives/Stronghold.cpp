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

bool AStronghold::GetRandomSpawnPoint(FVector& OutLocation, FRotator& OutRotation)
{
	int RandIndex = rand() % SpawnAreas.Num();

	UBoxComponent* SpawnArea = SpawnAreas[RandIndex];

	FVector PointLocation = UKismetMathLibrary::RandomPointInBoundingBox(SpawnArea->GetComponentLocation(), SpawnArea->GetScaledBoxExtent());


	// check if location is on the navmesh
	FNavLocation NavLocation;
	UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	bool navResult = NavigationArea->GetRandomReachablePointInRadius(PointLocation, 500.f, NavLocation);

	if (!navResult) {
		return false;
	}

	FNavLocation NewNavLocation;
	navResult = NavigationArea->ProjectPointToNavigation(NavLocation.Location, NewNavLocation);

	OutLocation = NewNavLocation.Location;
	OutRotation = SpawnArea->GetComponentRotation();

	return true;
}

void AStronghold::BeginPlay()
{
	Super::BeginPlay();

	DefaultSpawnRate = SpawnRate;

	DominantFaction = FOccupiedFaction();

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

	if (THandler_SpawnDelay.IsValid())
	{
		auto CurrentRate = GetWorldTimerManager().GetTimerRate(THandler_SpawnDelay);

		// Is there is an opponent in stronghold of occupied stronghold?
		auto NewSpawnRate = IsUnderAttack() ? SpawnRate * 5.f : SpawnRate;

		// only set timer if current rate is not equal to new rate.
		if (CurrentRate != NewSpawnRate)
		{
			GetWorldTimerManager().ClearTimer(THandler_SpawnDelay);
			GetWorldTimerManager().SetTimer(THandler_SpawnDelay, this, &AStronghold::SpawnDefender, NewSpawnRate, true);
		}
	}
}

void AStronghold::CheckOverlappingCombatatant()
{
	TArray<AActor*> OverlappingActors;
	StrongholdArea->GetOverlappingActors(OverlappingActors, ACombatCharacter::StaticClass());

	TArray<ACombatCharacter*> OccupyingCombatants;

	for (int i = 0; i < OverlappingActors.Num(); i++)
	{
		auto CombatCharacter = Cast<ACombatCharacter>(OverlappingActors[i]);

		if (!CombatCharacter) {
			continue;
		}

		if (!UHealthComponent::IsActorAlive(CombatCharacter)) {
			continue;
		}

		// avoid if in a vehicle or other non walking activities. Stronghold should only be affected by characters on foot.
		if (CombatCharacter->GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Walking) {
			return;
		}

		if (!OccupyingCombatants.Contains(CombatCharacter)) {
			OccupyingCombatants.Add(CombatCharacter);
		}
	}

	TotalOccupyingCombatants = OccupyingCombatants;
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
	if (DominantFaction.Faction == TeamFaction::Neutral) {
		return;
	}

	if (DefendingCombatatants.Num() >= SpawnMax) {
		UpdateDefenders();
		return;
	}

	if (DominantFaction.FactionDataSet == nullptr) {
		return;
	}

	if (DominantFaction.FactionDataSet->OperativeCharacterClass == nullptr) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector OutLocation;
	FRotator OutRotation;
	bool HasSpawnPoint = GetRandomSpawnPoint(OutLocation, OutRotation);

	if (!HasSpawnPoint) {
		return;
	}

	ABaseCharacter* Character = GetWorld()->SpawnActor<ABaseCharacter>(DominantFaction.FactionDataSet->OperativeCharacterClass, OutLocation, OutRotation, SpawnParams);

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

	TMap<TeamFaction, int> FactionMap;
	TArray<FOccupiedFaction> NewOccupiedFactions;

	for (int i = TotalOccupyingCombatants.Num() - 1; i >= 0; i--)
	{
		ACombatCharacter* Character = TotalOccupyingCombatants[i];

		if (Character)
		{
			auto TeamFactionComp = Cast<UTeamFactionComponent>(Character->GetComponentByClass(UTeamFactionComponent::StaticClass()));

			if (TeamFactionComp)
			{
				TeamFaction Faction = TeamFactionComp->GetSelectedFaction();

				int index = 0;
				// add faction to local array.
				if (GetFaction(NewOccupiedFactions, Faction, index).FactionDataSet == nullptr)
				{
					NewOccupiedFactions.Add(AddFaction(Character, Faction));
				}

				// is the faction already added to the map? if so, then increment the combatant count by 1.
				// & update the key
				if (FactionMap.Contains(Faction))
				{
					int Count = FactionMap[Faction];
					FactionMap.Add(Faction, Count + 1);
				}
				// if not, then add faction and set default count of 1. 
				else
				{
					FactionMap.Add(Faction, 1);
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


	// update the list of occupants.
	OccupiedFactions = NewOccupiedFactions;

	TArray<TeamFaction> OutKeys;
	FactionMap.GetKeys(OutKeys);

	for (TeamFaction Faction : OutKeys)
	{
		int index = 0;
		FOccupiedFaction OccupiedFaction = GetFaction(OccupiedFactions, Faction, index);

		if (OccupiedFaction.FactionDataSet != nullptr)
		{
			OccupiedFaction.FactionCount = FactionMap[Faction];
			OccupiedFactions[index] = OccupiedFaction;
		}
	}

	OnStrongholdUnderAttack.Broadcast(IsUnderAttack());
}

void AStronghold::UpdateDefenders()
{
	if (DefendingCombatatants.Num() <= 0) {
		return;
	}

	for (int i = DefendingCombatatants.Num() - 1; i >= 0; i--)
	{
		auto Character = DefendingCombatatants[i];

		// if dead or has been recruited by commander then it is no longer a defend of the stronghold 
		if (!UHealthComponent::IsActorAlive(Character) || Character->GetCommander() != nullptr)
		{
			RemoveDefender(Character);
		}
	}
}

FOccupiedFaction AStronghold::AddFaction(ACombatCharacter* Character, TeamFaction Faction)
{
	FOccupiedFaction OccupyingFaction = FOccupiedFaction();
	OccupyingFaction.Faction = Faction;
	OccupyingFaction.FactionCount = 1;
	OccupyingFaction.FactionDataSet = Character->GetFactionDataSet();
	OccupyingFaction.FactionRowName = Character->GetFactionRowName();
	OccupyingFaction.FlagMaterial = OccupyingFaction.FactionDataSet->FlagMaterial;
	return OccupyingFaction;
}

FOccupiedFaction AStronghold::GetFaction(TArray<FOccupiedFaction> Factions, TeamFaction Faction, int& index)
{
	if (Factions.Num() <= 0) {
		return FOccupiedFaction();
	}

	for (int i = 0; i < Factions.Num(); i++)
	{
		FOccupiedFaction OccupiedFaction = Factions[i];

		if (OccupiedFaction.Faction == Faction)
		{
			index = i;
			return OccupiedFaction;
		}
	}

	return FOccupiedFaction();
}

void AStronghold::GetHighestFaction()
{
	if (OccupiedFactions.Num() < 0) {
		return;
	}

	FOccupiedFaction OwningFaction = FOccupiedFaction();

	for (FOccupiedFaction OccupiedFaction : OccupiedFactions)
	{
		if (OccupiedFaction.FactionCount > OwningFaction.FactionCount)
		{
			OwningFaction = OccupiedFaction;
		}
	}

	// play the capture sound
	if (DominantFaction.Faction == TeamFaction::Neutral) 
	{
		if (CaptureSound != NULL)
		{
			StrongholdAudio->Sound = CaptureSound;
			StrongholdAudio->Play();
		}

		OnStrongholdCaptured.Broadcast(OwningFaction);
	}
	else
	{
		// Stronghold has been overrun if the dominant faction is no longer the same
		if (DominantFaction.Faction != OwningFaction.Faction)
		{
			if (OverrunSound != NULL)
			{
				StrongholdAudio->Sound = OverrunSound;
				StrongholdAudio->Play();
			}

			OnStrongholdCaptured.Broadcast(OwningFaction);
		}
	}

	// update the dominant faction
	DominantFaction = OwningFaction;
	FactionFlag->SetMaterial(FlagClothMaterialIndex, DominantFaction.FlagMaterial);
}

bool AStronghold::IsUnderAttack()
{
	return OccupiedFactions.Num() > 1;
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
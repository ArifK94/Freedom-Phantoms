#include "Props/Stronghold.h"

#include "Characters/CombatCharacter.h"
#include "Controllers/CombatAIController.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/CoverPointComponent.h"

#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"


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
	GetCoverPoints();

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
		ACombatCharacter* CombatCharacter = Cast<ACombatCharacter>(OverlappingActors[i]);
		UHealthComponent* HealthComponent = Cast<UHealthComponent>(CombatCharacter->GetComponentByClass(UHealthComponent::StaticClass()));

		if (HealthComponent && HealthComponent->IsAlive())
		{
			if (!DoesOccupantExist(CombatCharacter))
			{
				TotalOccupyingCombatants.Add(CombatCharacter);
			}
		}
	}
}

void AStronghold::GetSpawnAreas()
{
	SpawnAreas.Add(SpawnBox);
	for (UActorComponent* component : GetComponentsByTag(UBoxComponent::StaticClass(), SpawnAreaComponentTag))
	{
		UBoxComponent* SpawnArea = Cast<UBoxComponent>(component);
		if (SpawnArea)
		{
			SpawnAreas.Add(SpawnArea);
		}
	}
}

void AStronghold::GetCoverPoints()
{
	for (UActorComponent* component : GetComponentsByClass(UCoverPointComponent::StaticClass()))
	{
		UCoverPointComponent* CoverPointComp = Cast<UCoverPointComponent>(component);
		if (CoverPointComp)
		{
			CoverPointComponents.Add(CoverPointComp);
		}
	}
}

UCoverPointComponent* AStronghold::GetCoverPoint(AActor* OwningCharacter)
{
	if (CoverPointComponents.Num() <= 0) {
		return nullptr;
	}

	for (int i = 0; i < CoverPointComponents.Num(); i++)
	{
		UCoverPointComponent* CoverPointComp = CoverPointComponents[i];

		if (!CoverPointComp->GetOwner())
		{
			CoverPointComp->SetOccupant(OwningCharacter);
			return CoverPointComp;
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
	bool navResult = NavigationArea->ProjectPointToNavigation(Location, NavLocation);

	if (navResult)
	{
		ABaseCharacter* Character = GetWorld()->SpawnActor<ABaseCharacter>(DominantFaction->FactionDataSet->OperativeCharacterClass, Location, SpawnArea->GetComponentRotation(), SpawnParams);

		if (Character)
		{
			ACombatAIController* CombatAI = Cast<ACombatAIController>(Character->GetController());

			if (CombatAI) {
				CombatAI->SetGuardingStronghold(this);
			}

			DefendingCombatatants.Add(Cast<ACombatCharacter>(Character));
		}
	}
}

void AStronghold::UpdateTotalOccupants()
{
	if (TotalOccupyingCombatants.Num() <= 0) {
		return;
	}

	for (int i = 0; i < TotalOccupyingCombatants.Num(); i++)
	{
		ACombatCharacter* Character = TotalOccupyingCombatants[i];
		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(Character->GetComponentByClass(UHealthComponent::StaticClass()));

		if (Character && CurrentHealth && CurrentHealth->IsAlive())
		{
			FOccupiedFaction* OccupiedFaction = DoesFactionExist(CurrentHealth->GetSelectedFaction());
			if (OccupiedFaction != nullptr)
			{
				OccupiedFaction->FactionCount++;
			}
			else
			{
				AddFaction(Character, CurrentHealth->GetSelectedFaction());
			}
		}
		else
		{
			// else dead
			TotalOccupyingCombatants.RemoveAt(i);
		}
	}
}

void AStronghold::UpdateDefenders()
{
	if (DefendingCombatatants.Num() <= 0) {
		return;
	}

	for (int i = 0; i < DefendingCombatatants.Num(); i++)
	{
		ACombatCharacter* Character = DefendingCombatatants[i];

		if (Character)
		{
			UHealthComponent* CurrentHealth = Cast<UHealthComponent>(Character->GetComponentByClass(UHealthComponent::StaticClass()));

			// if dead or has been recruited by commander then it is no longer a stronghold defender
			if ((CurrentHealth && !CurrentHealth->IsAlive()) || Character->getCommander() != nullptr)
			{
				DefendingCombatatants.RemoveAt(i);
			}
		}
		else
		{
			DefendingCombatatants.RemoveAt(i);
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

	// play the capture sound
	if (DominantFaction->Faction == TeamFaction::Neutral) {
		if (CaptureSound != NULL)
		{
			StrongholdAudio->Sound = CaptureSound;
			StrongholdAudio->Play();
		}
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
		}
	}

	// update the dominant faction
	DominantFaction = OwningFaction;
	FactionFlag->SetMaterial(FlagClothMaterialIndex, DominantFaction->FlagMaterial);
}
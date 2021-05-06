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
	SpawnDelayMin = 0.f;
	SpawnDelayMax = .5f;

	SpawnAreaComponentTag = "SpawnArea";

	StrongholdArea->OnComponentBeginOverlap.AddDynamic(this, &AStronghold::OnBeginOverlap);
	StrongholdArea->OnComponentEndOverlap.AddDynamic(this, &AStronghold::OnEndOverlap);
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
	StartSpawn();
}

void AStronghold::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateFaction();
	GetHighestFaction();
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

void AStronghold::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != NULL)
	{
		ACombatCharacter* CombatCharacter = Cast<ACombatCharacter>(OtherActor);

		if (CombatCharacter)
		{
			if (!DoesOccupantExist(CombatCharacter))
			{
				OccupyingCharacters.Add(CombatCharacter);
			}
		}
	}
}

void AStronghold::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != NULL)
	{
		ACombatCharacter* CombatCharacter = Cast<ACombatCharacter>(OtherActor);

		if (CombatCharacter)
		{
			if (DoesOccupantExist(CombatCharacter))
			{
				OccupyingCharacters.Remove(CombatCharacter);
			}
		}
	}
}

void AStronghold::SpawnCharacter()
{
	// check if captured by a faction
	if (DominantFaction->Faction == TeamFaction::Neutral) {
		return;
	}

	if (CurrentSpawnedActors > SpawnMax) {
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

	for (int i = 0; i < SpawnAreas.Num(); i++)
	{
		UBoxComponent* SpawnArea = SpawnAreas[i];

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
				CurrentSpawnedActors++;

				ACombatAIController* CombatAI = Cast<ACombatAIController>(Character->GetController());

				if (CombatAI) {
					CombatAI->SetGuardingStronghold(this);
				}
			}
		}
	}
}


void AStronghold::StartSpawn()
{
	if (!THandler_SpawnDelay.IsValid()) {
		GetWorldTimerManager().SetTimer(THandler_SpawnDelay, this, &AStronghold::SpawnCharacter, SpawnRate, true, FMath::RandRange(SpawnDelayMin, SpawnDelayMax));
	}
}

void AStronghold::StopSpawn()
{
	GetWorldTimerManager().ClearTimer(THandler_SpawnDelay);
}

void AStronghold::UpdateFaction()
{
	if (OccupyingCharacters.Num() <= 0) {
		return;
	}

	for (ACombatCharacter* Character : OccupyingCharacters)
	{
		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(Character->GetComponentByClass(UHealthComponent::StaticClass()));

		if (CurrentHealth->IsAlive())
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
			OccupyingCharacters.Remove(Character);

			// update the spawn actor count
			CurrentSpawnedActors--;
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
	if (OccupyingCharacters.Num() <= 0) {
		return false;
	}

	for (ACombatCharacter* Character : OccupyingCharacters)
	{
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

			// reset spawned actors count
			CurrentSpawnedActors = 0;
		}
	}

	// update the dominant faction
	DominantFaction = OwningFaction;
	FactionFlag->SetMaterial(FlagClothMaterialIndex, DominantFaction->FlagMaterial);
}
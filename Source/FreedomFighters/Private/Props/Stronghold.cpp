#include "Props/Stronghold.h"

#include "Characters/CombatCharacter.h"
#include "CustomComponents/HealthComponent.h"
#include "Managers/FactionManager.h"

#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"

AStronghold::AStronghold()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<UBoxComponent>(TEXT("Root"));
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
	DominantFaction->FactionManager = nullptr;

	GetSpawnAreas();
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

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int i = 0; i < SpawnAreas.Num(); i++)
	{
		UBoxComponent* SpawnArea = SpawnAreas[i];

		FVector Location = UKismetMathLibrary::RandomPointInBoundingBox(SpawnArea->GetComponentLocation(), SpawnArea->GetScaledBoxExtent());

		ABaseCharacter* Character = GetWorld()->SpawnActor<ABaseCharacter>(DominantFaction->FactionManager->GetOperativeCharacterClass(), Location, SpawnArea->GetComponentRotation(), SpawnParams);

		if (Character)
		{
			CurrentSpawnedActors++;
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
	if (OccupyingCharacters.Num() > 0)
	{
		for (ACombatCharacter* Character : OccupyingCharacters)
		{
			UHealthComponent* CurrentHealth = Cast<UHealthComponent>(Character->GetComponentByClass(UHealthComponent::StaticClass()));

			if (CurrentHealth->IsAlive())
			{
				if (OccupiedFactions.Num() > 0)
				{
					for (FOccupiedFaction* OccupiedFaction : OccupiedFactions)
					{
						if (OccupiedFaction->Faction == CurrentHealth->GetSelectedFaction())
						{
							OccupiedFaction->FactionCount++;
						}
						else
						{
							AddFaction(Character, CurrentHealth->GetSelectedFaction());
						}
					}
				}
				else
				{
					AddFaction(Character, CurrentHealth->GetSelectedFaction());
				}
			}
			else
			{
				OccupyingCharacters.Remove(Character);
			}

		}
	}
}

void AStronghold::AddFaction(ACombatCharacter* Character, TeamFaction Faction)
{
	FOccupiedFaction* OccupyingFaction = new FOccupiedFaction;
	OccupyingFaction->Faction = Faction;
	OccupyingFaction->FactionCount = 1;
	OccupyingFaction->FactionManager = Character->getFactionObj();
	OccupyingFaction->FlagMaterial = OccupyingFaction->FactionManager->GetFlagMaterial();

	OccupiedFactions.Add(OccupyingFaction);
}

bool AStronghold::DoesFactionExist(TeamFaction Faction)
{
	if (OccupiedFactions.Num() < 0) {
		return false;
	}

	for (FOccupiedFaction* OccupiedFaction : OccupiedFactions)
	{
		if (OccupiedFaction->Faction == Faction)
		{
			return true;
		}
	}

	return false;
}

bool AStronghold::DoesOccupantExist(ACombatCharacter* Occupant)
{
	if (OccupyingCharacters.Num() < 0) {
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
			OwningFaction->FactionManager = OccupiedFaction->FactionManager;
		}
	}

	DominantFaction = OwningFaction;
	FactionFlag->SetMaterial(FlagClothMaterialIndex, DominantFaction->FlagMaterial);
}
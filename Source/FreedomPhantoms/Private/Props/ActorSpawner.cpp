// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/ActorSpawner.h"
#include "Characters/CombatCharacter.h"
#include "Weapons/Weapon.h"
#include "Controllers/CombatAIController.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

AActorSpawner::AActorSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>("Scene");
	RootComponent = Scene;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>("SpawnArea");
	SpawnArea->SetCollisionProfileName(TEXT("NoCollision"));
	SpawnArea->CanCharacterStepUpOn = ECB_No;
	SpawnArea->SetupAttachment(RootComponent);

	TriggerArea = CreateDefaultSubobject<UBoxComponent>("TriggerArea");
	TriggerArea->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerArea->CanCharacterStepUpOn = ECB_No;
	TriggerArea->SetupAttachment(RootComponent);

	SpawnRate = 1.f;
	SpawnFirstDelay = 0.f;
	FreeSpawnLimit = 1;

	SpawnOffset = FVector::ZeroVector;

	SpawnOnNav = true;
	FreeSpawn = false;
	SpawnAtStart = true;
}

void AActorSpawner::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(false);

	if (TriggeredByEnemy)
	{
		TriggerArea->OnComponentBeginOverlap.AddDynamic(this, &AActorSpawner::OnOverlapBegin);
	}
	else
	{
		TriggerArea->DestroyComponent();

		if (SpawnAtStart)
		{
			StartSpawnTimer();
		}
	}
}

void AActorSpawner::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	if (!InHealthParameters.AffectedHealthComponent->IsAlive())
	{
		RemoveSpawnedActor(InHealthParameters.DamagedActor);
		StartSpawnTimer();
	}
}

void AActorSpawner::OnSpawnActorDestroyed(AActor* DestroyedActor)
{
	RemoveSpawnedActor(DestroyedActor);
	StartSpawnTimer();
}

void AActorSpawner::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) {
		return;
	}


	if (IsEnemyFaction(OtherActor))
	{
		StartSpawnTimer();
		TriggerArea->DestroyComponent();
	}
}

void AActorSpawner::BeginSpawn()
{
	if (!GetWorld()) {
		return;
	}
	
	// Stop timer if spawn limit reached.
	if (HasReachedSpawnLimit()) {
		StopSpawnTimer();
		return;
	}

	AActor* SpawnedActor = nullptr;
	AActor* TargetPoint = nullptr;

	// If no free spawn, then this means the actor will have a target point to go to after spawning.
	if (!FreeSpawn)
	{
		TArray<AActor*> OutActors = GetTargetPoints();

		if (OutActors.Num() <= 0) {
			StopSpawnTimer();
			return;
		}

		TargetPoint = OutActors[FMath::RandRange(0, OutActors.Num() - 1)];

		if (!IsTargetPointValid(TargetPoint)) {
			return;
		}


		SpawnedActor = SpawnActor();

		if (!SpawnedActor) {
			return;
		}

		// Set NPC to target destination.
		AController* Controller = SpawnedActor->GetInstigator()->GetController();

		if (Controller)
		{
			ACombatAIController* AIController = Cast<ACombatAIController>(Controller);

			if (AIController)
			{
				AIController->SetPriorityDestination(TargetPoint->GetActorLocation());
			}
		}
	}

	// Destroy the target point as it will not longer be needed if not constantly spawning
	// otherwise set the owner of the target point to prevent this target point from being assigned more than once.
	if (ConstantSpawning)
	{
		if (!SpawnedActor) {
			SpawnedActor = SpawnActor();
		}

		if (!SpawnedActor) {
			return;
		}

		if (TargetPoint)
		{
			TargetPoint->SetOwner(SpawnedActor);
		}

		SpawnedActor->OnDestroyed.AddDynamic(this, &AActorSpawner::OnSpawnActorDestroyed);

		auto ActorComponent = SpawnedActor->GetComponentByClass(UHealthComponent::StaticClass());

		if (ActorComponent) 
		{
			auto HealthComponent = Cast<UHealthComponent>(ActorComponent);

			if (HealthComponent)
			{
				HealthComponent->OnHealthChanged.AddDynamic(this, &AActorSpawner::OnHealthUpdate);
			}
		}
	}
	else
	{
		if (TargetPoint)
		{
			TargetPoint->Destroy();
		}
	}

	SpawnedActors.Add(SpawnedActor);

	OnActorSpawned.Broadcast(this, SpawnedActor);
}

AActor* AActorSpawner::SpawnActor()
{
	if (!ActorClass && ActorClasses.IsEmpty()) {
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector Location = GetSpawnLocation();

	TSubclassOf<AActor> TargetActorClass = ActorClass ? ActorClass : ActorClasses[rand() % ActorClasses.Num()];

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(TargetActorClass, Location, FRotator::ZeroRotator, SpawnParams);

	if (!SpawnedActor) {
		return nullptr;
	}

	SpawnWeapon(SpawnedActor);

	return SpawnedActor;
}

FVector AActorSpawner::GetSpawnLocation()
{
	FVector PointLocation = UKismetMathLibrary::RandomPointInBoundingBox(SpawnArea->GetComponentLocation(), SpawnArea->GetScaledBoxExtent());

	// Set up line trace parameters
	FVector Start = PointLocation + FVector(0, 0, 1000); // Start above desired spawn point
	FVector End = PointLocation - FVector(0, 0, 1000);   // End below desired spawn point


	FHitResult HitResult;
	FCollisionQueryParams QueryParams;

	// Perform line trace
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

	if (bHit)
	{
		// Adjust spawn location to be slightly above the hit point
		return HitResult.Location + SpawnOffset; // 100 units above ground
	}

	return PointLocation + SpawnOffset;
}

void AActorSpawner::SpawnWeapon(AActor* Actor)
{
	if (!Actor || !CharacterWeaponClass) {
		return;
	}

	ACombatCharacter* Character = Cast<ACombatCharacter>(Actor);

	if (!Character) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(CharacterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (Weapon)
	{
		Character->SetPrimaryWeapon(Weapon);
	}
}

bool AActorSpawner::IsEnemyFaction(AActor* Actor)
{
	auto ActorComponent = Actor->GetComponentByClass(UTeamFactionComponent::StaticClass());

	if (!ActorComponent) {
		return false;
	}

	auto FactionComp = Cast<UTeamFactionComponent>(ActorComponent);

	if (!FactionComp) {
		return false;
	}

	return FactionComp->GetSelectedFaction() != FriendlyFaction;
}

bool AActorSpawner::IsTargetPointValid(AActor* TargetPointActor)
{
	return !UKismetSystemLibrary::IsValid(TargetPointActor->GetOwner()) || !UHealthComponent::IsActorAlive(TargetPointActor->GetOwner());
}

bool AActorSpawner::HasFreeTargetPoints()
{
	TArray<AActor*> OutActors = GetTargetPoints();

	for (AActor* Actor : OutActors)
	{
		if (!UKismetSystemLibrary::IsValid(Actor->GetOwner()))
		{
			return true;
		}
	}

	return false;
}

bool AActorSpawner::HasReachedSpawnLimit()
{
	return FreeSpawn && SpawnedActors.Num() >= FreeSpawnLimit;
}

TArray<AActor*> AActorSpawner::GetTargetPoints()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetPointTag, OutActors);
	return OutActors;
}

void AActorSpawner::RemoveSpawnedActor(AActor* Actor)
{
	if (SpawnedActors.Contains(Actor))
	{
		SpawnedActors.Remove(Actor);
	}
}

void AActorSpawner::StartSpawnTimer()
{
	if (!THandler_Spawn.IsValid()) {
		GetWorldTimerManager().SetTimer(THandler_Spawn, this, &AActorSpawner::BeginSpawn, SpawnRate, true, SpawnFirstDelay);
	}
}

void AActorSpawner::StopSpawnTimer()
{
	GetWorldTimerManager().ClearTimer(THandler_Spawn);
}

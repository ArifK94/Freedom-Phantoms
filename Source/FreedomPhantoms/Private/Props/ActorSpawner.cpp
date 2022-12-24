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
		TriggerArea->SetCollisionProfileName(TEXT("NoCollision"));
		StartSpawnTimer();
	}
}

void AActorSpawner::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	if (!InHealthParameters.AffectedHealthComponent->IsAlive())
	{
		StartSpawnTimer();
	}
}

void AActorSpawner::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) {
		return;
	}

	if (IsEnemyFaction(OtherActor))
	{
		StartSpawnTimer();
		TriggerArea->SetCollisionProfileName(TEXT("NoCollision"));
	}
}

void AActorSpawner::BeginSpawn()
{
	if (!GetWorld()) {
		return;
	}

	TArray<AActor*> OutActors = GetTargetPoints();

	if (OutActors.Num() <= 0) {
		StopSpawnTimer();
		return;
	}

	AActor* Actor = OutActors[FMath::RandRange(0, OutActors.Num() - 1)];

	if (!IsTargetPointValid(Actor)) {
		return;
	}

	AActor* SpawnedActor = SpawnActor();

	if (!SpawnedActor) {
		return;
	}

	AController* Controller = SpawnedActor->GetInstigator()->GetController();

	ACombatAIController* AIController = Cast<ACombatAIController>(Controller);

	if (!AIController) {
		return;
	}

	AIController->SetPriorityDestination(Actor->GetActorLocation());

	// Destroy the target point as it will not longer be needed if not constantly spawning
	// otherwise set the owner of the target point to prevent this target point from being assigned more than once.
	if (ConstantSpawning)
	{
		Actor->SetOwner(SpawnedActor);

		auto ActorComponent = SpawnedActor->GetComponentByClass(UHealthComponent::StaticClass());

		if (!ActorComponent) {
			return;
		}

		auto HealthComponent = Cast<UHealthComponent>(ActorComponent);

		if (HealthComponent)
		{
			HealthComponent->OnHealthChanged.AddDynamic(this, &AActorSpawner::OnHealthUpdate);
		}
	}
	else
	{
		Actor->Destroy();
	}
}

AActor* AActorSpawner::SpawnActor()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector Location;
	bool IsValid;
	LoadSpawnLocation(Location, IsValid);

	if (!IsValid) {
		return nullptr;
	}

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, Location, FRotator::ZeroRotator, SpawnParams);

	if (!SpawnedActor) {
		return nullptr;
	}

	SpawnWeapon(SpawnedActor);

	return SpawnedActor;
}

void AActorSpawner::LoadSpawnLocation(FVector& Location, bool& IsValid)
{
	FVector PointLocation = UKismetMathLibrary::RandomPointInBoundingBox(SpawnArea->GetComponentLocation(), SpawnArea->GetScaledBoxExtent());

	FNavLocation NavLocation;
	UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	IsValid = NavigationArea->ProjectPointToNavigation(PointLocation, NavLocation);

	Location = NavLocation.Location;
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

TArray<AActor*> AActorSpawner::GetTargetPoints()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetPointTag, OutActors);
	return OutActors;
}

void AActorSpawner::StartSpawnTimer()
{
	if (!THandler_Spawn.IsValid()) {
		GetWorldTimerManager().SetTimer(THandler_Spawn, this, &AActorSpawner::BeginSpawn, SpawnRate, true);
	}
}

void AActorSpawner::StopSpawnTimer()
{
	GetWorldTimerManager().ClearTimer(THandler_Spawn);
}

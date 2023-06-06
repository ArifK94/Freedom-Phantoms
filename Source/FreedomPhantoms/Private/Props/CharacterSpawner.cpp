// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/CharacterSpawner.h"

#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACharacterSpawner::ACharacterSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(false);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = false;
	RootComponent = CapsuleComponent;

#if WITH_EDITORONLY_DATA

	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->bTreatAsASprite = true;
		ArrowComponent->SetupAttachment(CapsuleComponent);
		ArrowComponent->bIsScreenSizeScaled = true;
		ArrowComponent->SetSimulatePhysics(false);
	}

#endif // WITH_EDITORONLY_DATA

	Priority = 0;
}

void ACharacterSpawner::BeginPlay()
{
	Super::BeginPlay();

	DefaultPriority = Priority;
}

void ACharacterSpawner::SetDefaultPriority()
{
	Priority = DefaultPriority;
}

bool ACharacterSpawner::GetSpawnTransform(UWorld* World, FVector& OutLocation, FRotator& OutRotation)
{
	if (!World) {
		UE_LOG(LogTemp, Error, TEXT("GetSpawnTransform No World found."));
		return false;
	}

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(World, ACharacterSpawner::StaticClass(), OutActors);

	TArray<ACharacterSpawner*> HighestPriorities;

	for (AActor* OutActor : OutActors)
	{
		ACharacterSpawner* CharacterSpawner = Cast<ACharacterSpawner>(OutActor);

		if (HighestPriorities.Num() <= 0)
		{
			HighestPriorities.Add(CharacterSpawner);
		}
		// if current iteration has equal priority than the last highest priority spawner, then add to the array.
		else if (CharacterSpawner->GetPriority() == HighestPriorities[HighestPriorities.Num() - 1]->GetPriority())
		{
			HighestPriorities.Add(CharacterSpawner);
		}
		// if current iteration has higher priority than the last highest priority spawner, then empty the array and add the highest.
		else if (CharacterSpawner->GetPriority() > HighestPriorities[HighestPriorities.Num() - 1]->GetPriority())
		{
			HighestPriorities.Empty();
			HighestPriorities.Add(CharacterSpawner);
		}
	}

	if (HighestPriorities.Num() == 0) {
		UE_LOG(LogTemp, Error, TEXT("GetSpawnTransform No HighestPriorities found."));
		return false;
	}

	int RandomIndex = rand() % HighestPriorities.Num();
	ACharacterSpawner* HighestPriority = HighestPriorities[RandomIndex];

	OutLocation = HighestPriority->GetActorLocation();
	OutRotation = HighestPriority->GetActorRotation();

	return true;

}

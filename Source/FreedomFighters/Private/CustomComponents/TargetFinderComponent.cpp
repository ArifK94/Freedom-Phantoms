// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"

#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"

UTargetFinderComponent::UTargetFinderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	TargetSightRadius = 7000.0f;
	FinderLimit = 3;
}


void UTargetFinderComponent::BeginPlay()
{
	Super::BeginPlay();

	Init();
}

void UTargetFinderComponent::Init()
{
	if (!GetOwner()) {
		return;
	}

	// Alternative to AI Sight Perception in case 360 sight is wanted
	if (TargetSightSphere == nullptr)
	{
		TargetSightSphere = NewObject<USphereComponent>(GetOwner());

		if (TargetSightSphere)
		{
			TargetSightSphere->RegisterComponent();
			TargetSightSphere->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			TargetSightSphere->SetSphereRadius(TargetSightRadius);
			TargetSightSphere->SetCollisionProfileName(TEXT("AITargetSight"));
		}
	}
}

AActor* UTargetFinderComponent::FindTarget()
{
	if (!TargetSightSphere) {
		Init();

		if (!TargetSightSphere) {
			return nullptr;
		}
	}

	AActor* TargetActor = nullptr;
	TArray<AActor*> ActorsInSight;
	float TargetSightDistance = TargetSightRadius;

	TargetSightSphere->GetOverlappingActors(ActorsInSight, UTeamFactionComponent::StaticClass());

	FVector OwnerLocation = GetOwner()->GetActorLocation();


	FVector EyeLocation;
	FRotator EyeRotation;
	GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	// The current limit of targets to process
	int CurrentProcessedCharacters = 0;

	for (int index = 0; index < ActorsInSight.Num(); index++)
	{
		if (CurrentProcessedCharacters > FinderLimit) {
			break;
		}

		AActor* PotentialEnemy = ActorsInSight[index];

		auto TeamFaction = Cast<UTeamFactionComponent>(PotentialEnemy->GetComponentByClass(UTeamFactionComponent::StaticClass()));

		if (TeamFaction && TeamFaction->GetSelectedFaction() != TeamFaction::Neutral)
		{
			bool IsFactionCompActive = UTeamFactionComponent::IsComponentActive(PotentialEnemy);
			bool IsEnemy = !UTeamFactionComponent::IsFriendly(GetOwner(), PotentialEnemy);

			// is target alive & an enemy
			if (IsFactionCompActive && IsEnemy)
			{
				FVector EnemyLocation = PotentialEnemy->GetActorLocation();


				// check if can see the target
				FHitResult HitTargetResult;

				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(GetOwner());
				QueryParams.bTraceComplex = false;

				FCollisionObjectQueryParams ObjectParams;
				ObjectParams.AllObjects;

				bool bTargetHit = GetWorld()->LineTraceSingleByObjectType(
					HitTargetResult,
					EyeLocation,
					EnemyLocation,
					ObjectParams,
					QueryParams);

				if (bTargetHit)
				{
					if (HitTargetResult.GetActor() == PotentialEnemy)
					{
						// get closest enemy
						float DistanceDiff = FVector::Dist(OwnerLocation, EnemyLocation);

						if (DistanceDiff < TargetSightDistance)
						{
							TargetSightDistance = DistanceDiff;
							TargetActor = PotentialEnemy;
						}
					}
				}

				CurrentProcessedCharacters++;

			}

		}
	}

	return TargetActor;
}


bool UTargetFinderComponent::IsTargetBehind(AActor* ActorA, AActor* TargetActor)
{
	if (ActorA == nullptr || TargetActor == nullptr) {
		return false;
	}

	FVector MGForwardPos = UKismetMathLibrary::GetForwardVector(ActorA->GetActorRotation());
	FVector Normalised = TargetActor->GetActorLocation() - ActorA->GetActorLocation();
	UKismetMathLibrary::Vector_Normalize(Normalised);
	float Angle = UKismetMathLibrary::Dot_VectorVector(MGForwardPos, Normalised);

	if (Angle < -0.7f)
	{
		return true;
	}

	return false;
}
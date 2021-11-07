// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "Characters/BaseCharacter.h"

#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"

UTargetFinderComponent::UTargetFinderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	TargetSightRadius = 7000.0f;
	FinderLimit = 5.f;
}


void UTargetFinderComponent::BeginPlay()
{
	Super::BeginPlay();

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
	AActor* TargetActor = nullptr;
	TArray<AActor*> ActorsInSight;
	float TargetSightDistance = TargetSightRadius;

	TargetSightSphere->GetOverlappingActors(ActorsInSight, ABaseCharacter::StaticClass());

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

		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(PotentialEnemy->GetComponentByClass(UHealthComponent::StaticClass()));

		if (CurrentHealth && CurrentHealth->IsAlive() && CurrentHealth->GetSelectedFaction() != TeamFaction::Neutral)
		{
			bool IsEnemy = !UHealthComponent::IsFriendly(GetOwner(), PotentialEnemy);

			// is target alive & an enemy
			if (IsEnemy)
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


bool UTargetFinderComponent::IsTargetBehind(AActor* TargetActor)
{
	if (TargetActor == nullptr) {
		return false;
	}

	FVector MGForwardPos = UKismetMathLibrary::GetForwardVector(GetOwner()->GetActorRotation());
	FVector Normalised = TargetActor->GetActorLocation() - GetOwner()->GetActorLocation();
	UKismetMathLibrary::Vector_Normalize(Normalised);
	float Angle = UKismetMathLibrary::Dot_VectorVector(MGForwardPos, Normalised);

	if (Angle < -0.7f)
	{
		return true;
	}

	return false;
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"

#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
//#include "Perception/AIPerceptionComponent.h"
//#include "Perception/AISenseConfig.h"
//#include "Perception/AISenseConfig_Sight.h"
//#include "Perception/AISense_Sight.h"

UTargetFinderComponent::UTargetFinderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	TargetSightRadius = 7000.0f;
	FinderLimit = 5;

	FindTargetPerFrame = false;
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

	if (!PerceptionComp)
	{
		//PerceptionComp = Cast<UAIPerceptionComponent>(GetOwner()->GetComponentByClass(UAIPerceptionComponent::StaticClass()));

		// Get AI Sight Config
		//UAISenseConfig* SightConfig = GetPerceptionSenseConfig(UAISense_Sight::StaticClass());
		//if (SightConfig)
		//{
		//	AISightConfig = Cast<UAISenseConfig_Sight>(SightConfig);
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Error, TEXT("SetSightRange: Config == nullptr"));
		//}
	}

	if (FindTargetPerFrame)
	{
		if (!THandler_TargetSearch.IsValid()) 
		{
			GetOwner()->GetWorldTimerManager().SetTimer(THandler_TargetSearch, this, &UTargetFinderComponent::FindTargetUpdate, 1.f, true);
		}
	}
}

void UTargetFinderComponent::FindTargetUpdate()
{
	FindTarget();
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

					CurrentProcessedCharacters++;

				}


			}

		}
	}

	OnTargetSearch.Broadcast(TargetActor);
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

//UAISenseConfig* UTargetFinderComponent::GetPerceptionSenseConfig(TSubclassOf<UAISense> SenseClass)
//{
//	UAISenseConfig* result = nullptr;
//
//	FAISenseID Id = UAISense::GetSenseID(SenseClass);
//	if (!Id.IsValid())
//	{
//		UE_LOG(LogTemp, Error, TEXT("GetPerceptionSenseConfig: Wrong Sense ID"));
//	}
//	else
//	{
//		result = PerceptionComp->GetSenseConfig(Id);
//	}
//
//	return result;
//}
//
//
//void UTargetFinderComponent::SetVisionAngle()
//{
//	if (AISightConfig == nullptr) {
//		return;
//	}
//
//	// Set Vision angle based whether character is in the helicopter
//	if (OwningCombatCharacter->GetIsInAircraft())
//	{
//		AISightConfig->PeripheralVisionAngleDegrees = 90.0f;
//	}
//	else
//	{
//		AISightConfig->PeripheralVisionAngleDegrees = 180.0f;
//	}
//
//	PerceptionComp->RequestStimuliListenerUpdate();
//
//}
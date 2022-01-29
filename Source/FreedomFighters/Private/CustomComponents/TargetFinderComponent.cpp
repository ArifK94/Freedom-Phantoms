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

	ClassFilters.Add(ACharacter::StaticClass());
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

bool UTargetFinderComponent::GetTrace(FHitResult& OutHit, FVector Start, FVector End)
{
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bTraceComplex = false;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;

	return GetWorld()->LineTraceSingleByObjectType(
		OutHit,
		Start,
		End,
		ObjectParams,
		QueryParams);
}

AActor* UTargetFinderComponent::FindTarget()
{
	if (!GetOwner()) {
		return nullptr;
	}

	if (!TargetSightSphere) {
		Init();

		if (!TargetSightSphere) {
			return nullptr;
		}
	}

	AActor* TargetActor = nullptr;
	TArray<AActor*> ActorsInSight;
	float TargetSightDistance = TargetSightRadius;

	// Get all overlapped actors based that have team faction component attached
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

		if (!PotentialEnemy) {
			continue;
		}

		// Filter the class
		if (!IsActorFiltered(PotentialEnemy)) {
			continue;
		}

		auto TeamFaction = Cast<UTeamFactionComponent>(PotentialEnemy->GetComponentByClass(UTeamFactionComponent::StaticClass()));

		// Does not have a faction component or is neutral?
		if (!TeamFaction || TeamFaction->GetSelectedFaction() == TeamFaction::Neutral) {
			continue;
		}

		bool IsAlive = UHealthComponent::IsAlive(PotentialEnemy);
		bool IsEnemy = !UTeamFactionComponent::IsFriendly(GetOwner(), PotentialEnemy);

		// is target alive & an enemy
		if (!IsAlive || !IsEnemy) {
			continue;
		}

		FVector EnemyLocation = PotentialEnemy->GetActorLocation();

		// check if can see the target
		FHitResult HitTargetResult;
		auto Trace = GetTrace(HitTargetResult, EyeLocation, EnemyLocation);

		if (!Trace) {
			continue;
		}


		if (HitTargetResult.GetActor() == PotentialEnemy)
		{
			// get closest enemy
			float DistanceDiff = FVector::Dist(OwnerLocation, EnemyLocation);

			if (DistanceDiff < TargetSightDistance)
			{
				TargetSightDistance = DistanceDiff;
				TargetActor = PotentialEnemy;
			}

			CurrentProcessedCharacters++;
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

bool UTargetFinderComponent::IsActorFiltered(AActor* Actor)
{
	if (ClassFilters.Num() <= 0) {
		return true;
	}

	for (int i = 0; i < ClassFilters.Num(); i++)
	{
		if (Actor->IsA(ClassFilters[i])) {
			return true;
		}
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
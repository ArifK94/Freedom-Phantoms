// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/MountedGunFinderComponent.h"
#include "Weapons/MountedGun.h"

#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"


UMountedGunFinderComponent::UMountedGunFinderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SearchRadius = 500.0f;
	SearchLimit = 3;
}


void UMountedGunFinderComponent::BeginPlay()
{
	Super::BeginPlay();

	if (SearchRadius <= 0.f) {
		SearchRadius = 500.0f;
	}

	DefaultSearchRadius = SearchRadius;

	Init();
}

void UMountedGunFinderComponent::Init()
{
	if (!GetOwner()) {
		return;
	}

	AController* Controller = Cast<AController>(GetOwner());

	if (!Controller) {
		return;
	}

	OwningPawn = Controller->GetPawn();

	if (!OwningPawn) {
		return;
	}

	// Alternative to AI Sight Perception in case 360 sight is wanted
	if (SearchSphere == nullptr)
	{
		SearchSphere = NewObject<USphereComponent>(OwningPawn);
		if (SearchSphere)
		{
			SearchSphere->RegisterComponent();
			SearchSphere->AttachToComponent(OwningPawn->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			SearchSphere->SetSphereRadius(SearchRadius);
			SearchSphere->SetGenerateOverlapEvents(true);
			SearchSphere->SetCanEverAffectNavigation(false);
			SearchSphere->SetCollisionProfileName(TEXT("OverlapAll"));
		}
	}

}

AMountedGun* UMountedGunFinderComponent::FindMG()
{
	AMountedGun* SelectedMG = nullptr;

	if (!SearchSphere) {
		Init();

		if (!SearchSphere) {
			return SelectedMG;
		}
	}

	float TargetSightDistance = SearchRadius;

	TArray<AActor*> ActorsInSight;
	SearchSphere->GetOverlappingActors(ActorsInSight, AMountedGun::StaticClass());

	// The current limit of targets to process
	int CurrentSearchCount = 0;

	for (int i = 0; i < ActorsInSight.Num(); i++)
	{
		if (CurrentSearchCount > SearchLimit) {
			break;
		}

		AMountedGun* PotentialMG = Cast<AMountedGun>(ActorsInSight[i]);

		if (PotentialMG)
		{
			//check if mounted gun is present in the stronghold and has no owner as well as no potential owner in case another AI wishes to use it
			bool HasNoOwner = PotentialMG->GetOwner() == nullptr && PotentialMG->GetPotentialOwner() == nullptr;

			if (HasNoOwner && PotentialMG->GetCanTraceInteraction())
			{
				float DistanceDiff = FVector::Dist(OwningPawn->GetActorLocation(), PotentialMG->GetActorLocation());

				if (DistanceDiff < TargetSightDistance)
				{
					TargetSightDistance = DistanceDiff;
					SelectedMG = PotentialMG;
				}

				CurrentSearchCount++;
			}
		}
	}

	return SelectedMG;
}

bool UMountedGunFinderComponent::IsInTargetRange(AMountedGun* MG, FVector StartLocation, FVector TargetLocation)
{
	if (!MG) {
		return true;
	}

	FVector Start = StartLocation - TargetLocation;
	//Start = UKismetMathLibrary::InverseTransformDirection(OwningCombatCharacter->FollowCamera->GetComponentTransform(), Start);
	FRotator TargetRot = UKismetMathLibrary::MakeRotFromX(Start);

	TargetRot = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);

	bool YawRange = UKismetMathLibrary::InRange_FloatFloat(TargetRot.Yaw, MG->GetYawMin(), MG->GetYawMax(), false, false);
	bool PitchRange = UKismetMathLibrary::InRange_FloatFloat(TargetRot.Pitch, MG->GetPitchMin(), MG->GetPitchMax(), false, false);

	if (YawRange && PitchRange)
	{
		return true;
	}

	return true;
}

bool UMountedGunFinderComponent::IsInTargetRange(AMountedGun* MG, AActor* StartActor, AActor* TargetActor)
{
	if (!MG || !StartActor || !TargetActor) {
		return true;
	}

	return IsInTargetRange(MG, StartActor->GetActorLocation(), TargetActor->GetActorLocation());
}

void UMountedGunFinderComponent::FocusTarget(AMountedGun* MG, FVector Location)
{
	auto TargetRotation = FRotator::ZeroRotator;

	FVector EyeLocation;
	FRotator EyeRotation;
	GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	auto TargetLocation = Location - EyeLocation;
	auto T = MG->GetMGBaseTransform();
	auto TargetDirectionInvert = UKismetMathLibrary::InverseTransformDirection(T, TargetLocation);
	TargetRotation = UKismetMathLibrary::MakeRotFromX(TargetDirectionInvert);

	auto TargetRot = UKismetMathLibrary::RInterpTo(MG->GetRotationInput(), TargetRotation, GetWorld()->DeltaTimeSeconds, 1.5f);
	MG->SetRotationInput(TargetRot);
}

void UMountedGunFinderComponent::FocusTarget(AActor* Owner, AMountedGun* MG, FVector Location)
{
	auto TargetRotation = FRotator::ZeroRotator;

	FVector EyeLocation;
	FRotator EyeRotation;
	Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	auto TargetLocation = Location - EyeLocation;
	auto T = MG->GetMGBaseTransform();
	auto TargetDirectionInvert = UKismetMathLibrary::InverseTransformDirection(T, TargetLocation);
	TargetRotation = UKismetMathLibrary::MakeRotFromX(TargetDirectionInvert);

	auto TargetRot = UKismetMathLibrary::RInterpTo(MG->GetRotationInput(), TargetRotation, Owner->GetWorld()->DeltaTimeSeconds, 1.5f);
	MG->SetRotationInput(TargetRot);
}

void UMountedGunFinderComponent::ResetSearchRadius()
{
	SearchRadius = DefaultSearchRadius;
}
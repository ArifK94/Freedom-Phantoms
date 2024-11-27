// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/RappellerComponent.h"
#include "Accessories/Rope.h"

#include "GameFramework/Actor.h"
#include "CableComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"


URappellerComponent::URappellerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	StartPosition = FVector::ZeroVector;
	EndPosition = FVector::ZeroVector;
	RappelDuration = 5.f;

	CurrentBoneIndex = 0;

	DestroyAfterRapelled = true;
}

void URappellerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (RappelCurve)
	{
		FOnTimelineFloat RappelProgressFunction;
		RappelProgressFunction.BindUFunction(this, FName("UpdateRappelProgress"));
		CurveTimeline.AddInterpFloat(RappelCurve, RappelProgressFunction);

		FOnTimelineEvent RappelFinishedFunction;
		RappelFinishedFunction.BindUFunction(this, FName("OnRappelFinished"));
		//CurveTimeline.SetTimelineFinishedFunc(RappelFinishedFunction);

		if (Rope)
		{
			InitializeRappel(Rope);
		}
	}
}

void URappellerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (RappelCurve)
	{
		CurveTimeline.TickTimeline(DeltaTime);
	}
}

void URappellerComponent::InitializeRappel(ARope* RopeActor)
{
	Rope = RopeActor;
	CurrentBoneIndex = 0;

	if (Rope && RappelCurve)
	{
		Rope->AttachActorToRope(GetOwner());

		CurveTimeline.SetLooping(false);
		CurveTimeline.SetPlayRate(1 / RappelDuration);
		CurveTimeline.PlayFromStart();
	}
}

void URappellerComponent::UpdateRappelProgress(float Value)
{
	if (GetOwner())
	{
		HandleRappelling(Value);

		FRappellingParameters RappellingParams;
		RappellingParams.IsRappelling = true;
		OnRappelChanged.Broadcast(RappellingParams);
	}
}

void URappellerComponent::OnRappelFinished()
{
	GetOwner()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Rope->DettachActorFromRope();

	CurrentBoneIndex = 0;

	FRappellingParameters RappellingParams;
	RappellingParams.IsComplete = true;
	OnRappelChanged.Broadcast(RappellingParams);

	if (DestroyAfterRapelled)
	{
		DestroyComponent();
	}
}

void URappellerComponent::HandleRappelling(float DeltaTime)
{
	if (!Rope) return;

	// Get the number of bones in the rope
	int32 NumBones = Rope->GetNumBones();
	if (NumBones == 0) return;

	// Calculate the new bone index based on rappel speed
	float DistanceToMove = RappelDuration * DeltaTime;
	FVector CurrentBoneLocation = Rope->GetBoneLocation(CurrentBoneIndex);

	while (DistanceToMove > 0 && CurrentBoneIndex < NumBones - 1)
	{
		FVector NextBoneLocation = Rope->GetBoneLocation(CurrentBoneIndex + 1);
		float BoneDistance = FVector::Dist(CurrentBoneLocation, NextBoneLocation);

		if (DistanceToMove >= BoneDistance)
		{
			DistanceToMove -= BoneDistance;
			CurrentBoneIndex++;
			CurrentBoneLocation = NextBoneLocation;
		}
		else
		{
			break;
		}
	}

	// Update character location
	if (Rope && CurrentBoneIndex < NumBones)
	{
		FVector TargetLocation = Rope->GetBoneLocation(CurrentBoneIndex);
		GetOwner()->SetActorLocation(TargetLocation);

		// Compute the yaw rotation to align with the rope direction
		FVector NextBoneLocation = Rope->GetBoneLocation(CurrentBoneIndex + 1);

		FVector Direction = NextBoneLocation - CurrentBoneLocation;
		Direction.Z = 0; // Ignore pitch (vertical direction) to constrain rotation to yaw
		FRotator TargetYawRotation = Direction.Rotation();

		// Apply only the yaw rotation
		FRotator CurrentRotation = GetOwner()->GetActorRotation();
		CurrentRotation.Yaw = TargetYawRotation.Yaw;
		GetOwner()->SetActorRotation(CurrentRotation);
	}

	// If the character reaches the end of the rope, stop rappelling
	if (CurrentBoneIndex >= NumBones - 1)
	{
		OnRappelFinished();
	}
}

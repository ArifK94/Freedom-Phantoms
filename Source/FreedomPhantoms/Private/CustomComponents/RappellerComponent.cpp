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
		CurveTimeline.SetTimelineFinishedFunc(RappelFinishedFunction);

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

	if (Rope && RappelCurve)
	{
		Rope->AttachActorToRope(GetOwner());
		StartPosition = Rope->GetStartLocation();
		EndPosition = Rope->GetEndLocation();

		CurveTimeline.SetLooping(false);
		CurveTimeline.SetPlayRate(1 / RappelDuration);
		CurveTimeline.PlayFromStart();
	}
}

void URappellerComponent::UpdateRappelProgress(float Value)
{
	FVector NewPosition = FMath::Lerp(StartPosition, EndPosition, Value);
	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->SetActorLocation(NewPosition);

		FRappellingParameters RappellingParams;
		RappellingParams.IsRappelling = true;
		OnRappelChanged.Broadcast(RappellingParams);
	}
}

void URappellerComponent::OnRappelFinished()
{
	FRappellingParameters RappellingParams;
	RappellingParams.IsComplete = true;
	OnRappelChanged.Broadcast(RappellingParams);
}
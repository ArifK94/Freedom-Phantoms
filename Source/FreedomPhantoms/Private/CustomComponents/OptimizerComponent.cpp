// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/OptimizerComponent.h"
#include "Services/SharedService.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UOptimizerComponent::UOptimizerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	BeginPlayDelayAmount = 5.f;
	TickRadius = 1000.f;
	TickIntervalOptimized = .5f;

	ApplyDistanceOptimization = false;
	MinimumDistanceOptimization = 5000.f;

	CanOptimizeTick = true;
	CanOptimizeChildrenComponents = true;
}


void UOptimizerComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(false);

	//if (GetOwner()) {
	//	GetOwner()->GetWorldTimerManager().SetTimer(THandler_BeginPlay, this, &UOptimizerComponent::BeginPlayDelay, 1.f, false, BeginPlayDelayAmount);
	//}

}

void UOptimizerComponent::BeginPlayDelay()
{
	SetComponentTickEnabled(true);

	GetOwner()->GetWorldTimerManager().ClearTimer(THandler_BeginPlay);
}

void UOptimizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//if (!GetOwner() || !GetWorld()) {
	//	return;
	//}

	//if (CanOptimizeTick)
	//{
	//	float NewTickInterval = 0.f;

	//	GetOptimizedTick(NewTickInterval);

	//	OptimizeActorTick(NewTickInterval);

	//	if (CanOptimizeChildrenComponents)
	//	{
	//		OptimizeComponentsTick(NewTickInterval);
	//	}
	//}
}


void UOptimizerComponent::GetOptimizedTick(float& NewTickInterval)
{
	// Actors near the player camera should have normal tick intervals to avoid experiencing the lag of an actor when player rotates camera around fast.
	auto PlayerCam = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

	bool IsNearPlayerCam = false;
	bool IsInDistance = false;

	if (PlayerCam)
	{
		IsNearPlayerCam = USharedService::IsNearTargetPosition(PlayerCam->GetViewTarget(), GetOwner(), TickRadius);
	}

	bool IsActorOnScreen = USharedService::IsActorOnScreen(GetWorld(), GetOwner(), OptimizeOffset);

	if (IsActorOnScreen && ApplyDistanceOptimization)
	{
		IsInDistance = USharedService::IsNearTargetPosition(PlayerCam->GetViewTarget(), GetOwner(), MinimumDistanceOptimization);
	}

	// If on screen, then set tick interval to default otherwise add more intervals in order to get good performance.
	NewTickInterval = IsActorOnScreen || IsNearPlayerCam || IsInDistance ? 0.f : TickIntervalOptimized;
}

void UOptimizerComponent::OptimizeActorTick(float NewTickInterval)
{
	// no need to run this function if owner is not ticking.
	if (!GetOwner()->IsActorTickEnabled()) {
		return;
	}

	// Save performance by avoiding following codes.
	if (GetOwner()->GetActorTickInterval() == NewTickInterval) {
		return;
	}

	GetOwner()->SetActorTickInterval(NewTickInterval);
}

void UOptimizerComponent::OptimizeComponentsTick(float NewTickInterval)
{
	auto Components = GetOwner()->GetComponents();

	for (auto Component : Components)
	{
		if (Component && Component != this && !IgnoredComponentClasses.Contains(Component->GetClass()))
		{
			Component->SetComponentTickInterval(NewTickInterval);
		}
	}

}

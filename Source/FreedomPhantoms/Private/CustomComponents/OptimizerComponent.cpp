// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/OptimizerComponent.h"
#include "Services/SharedService.h"

#include "Kismet/GameplayStatics.h"

UOptimizerComponent::UOptimizerComponent()
{
	TickRadius = 500.f;
	TickIntervalOptimized = .5f;

	CanOptimizeTick = true;
	CanOptimizeChildrenComponents = true;
}

void UOptimizerComponent::Init()
{
	Super::Init();

	if (GetOwner() && !HasInit)
	{
		DefaultActorTickInterval = GetOwner()->GetActorTickInterval();
		HasInit = true;
	}
}

void UOptimizerComponent::TimerTick()
{
	Super::TimerTick();

	if (!GetOwner() || !GetWorld()) {
		return;
	}

	if (CanOptimizeTick)
	{
		float NewTickInterval = 0.f;

		bool HasNewTick = GetOptimizedTick(NewTickInterval);

		if (HasNewTick)
		{
			OptimizeActorTick(NewTickInterval);

			if (CanOptimizeChildrenComponents)
			{
				OptimizeComponentsTick(NewTickInterval);
			}
		}
	}
}

bool UOptimizerComponent::GetOptimizedTick(float& NewTickInterval)
{
	// Actors near the player camera should have normal tick intervals to avoid experiencing the lag of an actor when player rotates camera around fast.
	auto PlayerCam = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

	bool IsNearPlayerCam = false;

	if (PlayerCam)
	{
		IsNearPlayerCam = USharedService::IsNearTargetPosition(PlayerCam->GetViewTarget(), GetOwner(), TickRadius);
	}

	bool IsActorOnScreen = USharedService::IsActorOnScreen(GetWorld(), GetOwner());

	// If on screen, then set tick interval to default otherwise add more intervals in order to get good performance.
	NewTickInterval = IsActorOnScreen || IsNearPlayerCam ? DefaultActorTickInterval : TickIntervalOptimized;

	return true;
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

// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/Engine/MyActorComponent.h"
#include "Characters/BaseCharacter.h"

#include "Kismet/GameplayStatics.h"

UMyActorComponent::UMyActorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	HasInit = false;
}

void UMyActorComponent::BeginPlay()
{
	Super::BeginPlay();

	StartTickTimer();
}

/**
* Owner can be Actor or Controller.
*/
void UMyActorComponent::BeginInit()
{
	if (GetOwner())
	{
		Init();

		// after x seconds, stop calling init as this function would not to be called
		if (!THandler_StopInit.IsValid() && GetWorld())
		{
			GetOwner()->GetWorldTimerManager().SetTimer(THandler_StopInit, this, &UMyActorComponent::StopInitTimer, 1.f, false, 10.f);
		}
	}
}

void UMyActorComponent::BeginTick()
{
	WorldDeltaSeconds = UGameplayStatics::GetWorldDeltaSeconds(GetWorld());

	if (GetOwner() && GetWorld() && !HasInit)
	{
		GetOwner()->GetWorldTimerManager().SetTimer(THandler_BeginInit, this, &UMyActorComponent::BeginInit, .2f, true);
	}

	TimerTick();
}

void UMyActorComponent::TimerTick()
{
}

void UMyActorComponent::Init()
{
	if (GetOwner())
	{
		// get the pawn actor.
		if (!OwningPawn)
		{
			// Assume the owner is a controller
			OwningController = Cast<AController>(GetOwner());

			// get the pawn from the controller.
			if (OwningController)
			{
				OwningPawn = OwningController->GetPawn();
			}
			// otherwise, the owner is an actor.
			else
			{
				OwningPawn = Cast<AActor>(GetOwner());
			}
		}

		// get the controller if already not assigned.
		if (!OwningController && OwningPawn)
		{
			OwningController = OwningPawn->GetInstigatorController();
		}

		// get the pawn owner from the controller if pawn owner is null.
		if (!OwningPawn && OwningController)
		{
			OwningPawn = OwningController->GetPawn();
		}

		if (OwningPawn)
		{
			OwningCharacter = Cast<ABaseCharacter>(OwningPawn);
		}
	}
}

void UMyActorComponent::StopInitTimer()
{
	if (!GetOwner() || !GetWorld()) {
		return;
	}

	HasInit = true;

	GetOwner()->GetWorldTimerManager().ClearTimer(THandler_BeginInit);
	GetOwner()->GetWorldTimerManager().ClearTimer(THandler_StopInit);
}


void UMyActorComponent::StartTickTimer(bool OverrideTimer)
{
	if (!GetOwner() || !GetWorld()) {
		return;
	}

	//float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(GetWorld());

	//// if running from beginplay, then delta secs would be zero.
	//DeltaTime = DeltaTime <= 0.f ? .5f : DeltaTime;

	if (OverrideTimer)
	{
		StopTickTimer();
	}

	if (!THandler_TimerTick.IsValid())
	{
		GetOwner()->GetWorldTimerManager().SetTimer(THandler_TimerTick, this, &UMyActorComponent::BeginTick, .5f, true);
	}
}

void UMyActorComponent::StartTickTimer(float InRate, bool OverrideTimer)
{
	if (!GetOwner() || !GetWorld()) {
		return;
	}

	if (OverrideTimer)
	{
		StopTickTimer();
	}

	if (!THandler_TimerTick.IsValid())
	{
		GetOwner()->GetWorldTimerManager().SetTimer(THandler_TimerTick, this, &UMyActorComponent::BeginTick, InRate, true);
	}
}

void UMyActorComponent::StopTickTimer()
{
	if (!GetOwner() || !GetWorld()) {
		return;
	}

	if (THandler_TimerTick.IsValid())
	{
		GetOwner()->GetWorldTimerManager().ClearTimer(THandler_TimerTick);
	}
}
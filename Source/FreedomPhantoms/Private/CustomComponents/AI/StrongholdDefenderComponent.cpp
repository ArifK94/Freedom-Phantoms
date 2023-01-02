// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/AI/StrongholdDefenderComponent.h"
#include "CustomComponents/CoverPointComponent.h"
#include "Objectives/Stronghold.h"

UStrongholdDefenderComponent::UStrongholdDefenderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStrongholdDefenderComponent::BeginPlay()
{
	Super::BeginPlay();

	//GetOwner()->GetWorldTimerManager().SetTimer(THandler_StrongholdCoverPoint, this, &UStrongholdDefenderComponent::FindDefenderPoint, 1.0f, true);
}


void UStrongholdDefenderComponent::FindDefenderPoint()
{
	if (Stronghold == nullptr) {
		return;
	}

	auto NewCoverPoint = Stronghold->GetCoverPoint(GetOwner());
	bool MoveToCoverPoint = false;

	if (NewCoverPoint) {

		// if already has a chosen cover point
		if (ChosenCoverPointComponent) {
			// if found a new priroity cover point then take it.
			if (NewCoverPoint->GetIsPriority()) {

				ChosenCoverPointComponent = NewCoverPoint;
				MoveToCoverPoint = true;

				ClearTimer();
			}
		}
		else {
			ChosenCoverPointComponent = NewCoverPoint;
			MoveToCoverPoint = true;
		}
	}


	if (ChosenCoverPointComponent) {

		FStrongholdDefenderParams StrongholdDefenderParams = FStrongholdDefenderParams();
		StrongholdDefenderParams.TargetPoint = ChosenCoverPointComponent->GetComponentLocation();
		OnDefenderPointFound.Broadcast(StrongholdDefenderParams);
	}
}

void UStrongholdDefenderComponent::ClearTimer()
{
	GetOwner()->GetWorldTimerManager().ClearTimer(THandler_StrongholdCoverPoint);
}

void UStrongholdDefenderComponent::SetDefender(AActor* Defender, AStronghold* Stronghold)
{
	if (!Defender || Defender->GetName() == "None") {
		return;
	}

	TWeakObjectPtr<AActor> TempActor = Defender;
	if (!TempActor.IsValid()) {
		return;
	}

	auto ActorComponent = TempActor->GetComponentByClass(UStrongholdDefenderComponent::StaticClass());

	if (!ActorComponent) {
		return;
	}

	auto StrongholdComponent = Cast<UStrongholdDefenderComponent>(ActorComponent);

	if (!StrongholdComponent) {
		return;
	}

	StrongholdComponent->SetStronghold(Stronghold);
}

void UStrongholdDefenderComponent::RemoveStronghold()
{
	if (Stronghold == nullptr) {
		return;
	}

	if (ChosenCoverPointComponent) {
		ChosenCoverPointComponent->SetOccupant(nullptr);
	}

	Stronghold->RemoveDefender(GetOwner());
	Stronghold = nullptr;
	ClearTimer();
}
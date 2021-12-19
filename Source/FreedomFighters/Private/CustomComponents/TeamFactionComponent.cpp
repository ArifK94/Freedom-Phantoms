// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/TeamFactionComponent.h"

UTeamFactionComponent::UTeamFactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SelectedFaction = TeamFaction::Neutral;

	IsCompActive = true;
}

bool UTeamFactionComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		// Assume Friendly
		return true;
	}

	auto FactionCompA = Cast<UTeamFactionComponent>(ActorA->GetComponentByClass(UTeamFactionComponent::StaticClass()));
	auto FactionCompB = Cast<UTeamFactionComponent>(ActorB->GetComponentByClass(UTeamFactionComponent::StaticClass()));


	if (FactionCompA == nullptr || FactionCompB == nullptr)
	{
		// Assume Friendly
		return true;
	}

	return FactionCompA->GetSelectedFaction() == FactionCompB->GetSelectedFaction();
}

bool UTeamFactionComponent::IsComponentActive(AActor* Owner)
{
	if (!Owner) {
		return false;
	}

	auto FactionComp = Cast<UTeamFactionComponent>(Owner->GetComponentByClass(UTeamFactionComponent::StaticClass()));

	if (!FactionComp) {
		return false;
	}

	return FactionComp->GetIsCompActive();
}

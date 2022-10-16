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

	// If same actor
	if (ActorA == ActorB) {
		return true;
	}

	auto ActorComponentA = ActorA->GetComponentByClass(UTeamFactionComponent::StaticClass());
	auto ActorComponentB = ActorB->GetComponentByClass(UTeamFactionComponent::StaticClass());

	if (ActorComponentA == nullptr || ActorComponentB == nullptr)
	{
		// Assume Non-Friendly (Neutral included.)
		return false;
	}


	auto FactionCompA = Cast<UTeamFactionComponent>(ActorComponentA);
	auto FactionCompB = Cast<UTeamFactionComponent>(ActorComponentB);


	if (FactionCompA == nullptr || FactionCompB == nullptr)
	{
		// Assume Friendly
		return true;
	}

	return FactionCompA->GetSelectedFaction() == FactionCompB->GetSelectedFaction();
}

bool UTeamFactionComponent::IsComponentActive(AActor* Owner)
{
	if (!Owner || Owner->GetName() == "None") {
		return false;
	}

	auto ActorComponent = Owner->GetComponentByClass(UTeamFactionComponent::StaticClass());

	if (!ActorComponent) {
		return false;
	}

	auto FactionComp = Cast<UTeamFactionComponent>(ActorComponent);

	if (!FactionComp) {
		return false;
	}

	return FactionComp->GetIsCompActive();
}

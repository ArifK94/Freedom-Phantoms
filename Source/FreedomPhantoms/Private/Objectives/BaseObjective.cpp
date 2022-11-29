// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/BaseObjective.h"
#include "Controllers/CustomPlayerController.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseObjective::ABaseObjective()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxCollider->SetCollisionProfileName(TEXT("Objective"));
	BoxCollider->CanCharacterStepUpOn = ECB_No;
	BoxCollider->SetupAttachment(RootComponent);

	IsObjectiveComplete = false;
	IsFinalObjective = false;

	Progress = 0.0f;
}

void ABaseObjective::ObjectiveComplete()
{
	BoxCollider->SetCollisionProfileName(TEXT("NoCollision"));

	if (ObjectiveCompleteSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ObjectiveCompleteSound);
	}

	IsObjectiveComplete = true;

	OnObjectiveCompleted.Broadcast(this);
	OnObjectiveUpdate.Broadcast(this, 1.0f);
}
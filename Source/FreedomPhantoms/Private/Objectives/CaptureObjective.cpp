// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/CaptureObjective.h"
#include "Controllers/CustomPlayerController.h"
#include "CustomComponents/HealthComponent.h"
#include "Managers/GameStateBaseCustom.h"
#include "Managers/ObjectiveManager.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ACaptureObjective::ACaptureObjective()
{
	CaptureRate = 1.0f;

	IsPlayerCapturing = false;
}

void ACaptureObjective::BeginPlay()
{
	Super::BeginPlay();

	GameStateBaseCustom = Cast<AGameStateBaseCustom>(UGameplayStatics::GetGameState(GetWorld()));

	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ACaptureObjective::OnOverlapBegin);
	BoxCollider->OnComponentEndOverlap.AddDynamic(this, &ACaptureObjective::OnOverlapEnd);
}

void ACaptureObjective::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsObjectiveComplete) {
		return;
	}

	// only check if player is not in capture position
	if (IsPlayerCapturing) {
		return;
	}

	if (OtherActor)
	{
		APawn* Pawn = Cast<APawn>(OtherActor);

		if (Pawn == nullptr) {
			return;
		}

		if (Pawn->IsPlayerControlled())
		{
			HealthComponent = Cast<UHealthComponent>(Pawn->GetComponentByClass(UHealthComponent::StaticClass()));

			if (HealthComponent && HealthComponent->IsAlive())
			{
				ACustomPlayerController* CustomPlayerController = Cast<ACustomPlayerController>(Pawn->GetController());

				if (CustomPlayerController)
				{
					GameStateBaseCustom->GetObjectiveManager()->SetCurrentMissionObjective(this);
					IsPlayerCapturing = true;
				}

				GetWorldTimerManager().SetTimer(THandler_CaptureProgress, this, &ACaptureObjective::UpdateCaptureProgress, CaptureRate, true);
			}
		}
	}
}


void ACaptureObjective::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// if player has left the capture point
	if (OtherActor)
	{
		APawn* Pawn = Cast<APawn>(OtherActor);

		if (Pawn == nullptr) {
			return;
		}

		if (Pawn->IsPlayerControlled())
		{
			IsPlayerCapturing = false;

			HealthComponent = nullptr;

			GetWorldTimerManager().ClearTimer(THandler_CaptureProgress);
		}
	}
}

void ACaptureObjective::UpdateCaptureProgress()
{
	if (!IsPlayerCapturing) {
		GetWorldTimerManager().ClearTimer(THandler_CaptureProgress);
		return;
	}

	if (HealthComponent && !HealthComponent->IsAlive()) {
		GetWorldTimerManager().ClearTimer(THandler_CaptureProgress);
		return;
	}

	if (Progress < 1.0f)
	{
		Progress += .1f;
	}

	if (Progress >= 1.0f) // Objective Complete!
	{
		Progress = 1.0f;
		IsPlayerCapturing = false;

		ObjectiveComplete();

		GetWorldTimerManager().ClearTimer(THandler_CaptureProgress);
	}

	OnObjectiveUpdate.Broadcast(this, Progress);

}

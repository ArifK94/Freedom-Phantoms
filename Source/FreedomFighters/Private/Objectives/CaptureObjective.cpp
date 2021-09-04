// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/CaptureObjective.h"
#include "Controllers/CustomPlayerController.h"

#include "Components/BoxComponent.h"

ACaptureObjective::ACaptureObjective()
{
	CaptureRate = 1.0f;

	IsPlayerCapturing = false;
}

void ACaptureObjective::BeginPlay()
{
	Super::BeginPlay();

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
			IsPlayerCapturing = true;

			ACustomPlayerController* CustomPlayerController = Cast<ACustomPlayerController>(Pawn->GetController());

			if (CustomPlayerController)
			{
				CustomPlayerController->SetCurrentMissionObjective(this);
			}

			GetWorldTimerManager().SetTimer(THandler_CaptureProgress, this, &ACaptureObjective::UpdateCaptureProgress, CaptureRate, true);
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

			GetWorldTimerManager().ClearTimer(THandler_CaptureProgress);
		}
	}
}

void ACaptureObjective::UpdateCaptureProgress()
{
	if (!IsPlayerCapturing) {
		return;
	}

	if (Progress < 1.0f)
	{
		Progress += .1f;
	}
	else // Objective Complete!
	{
		GetWorldTimerManager().ClearTimer(THandler_CaptureProgress);

		Progress = 1.0f;
		IsPlayerCapturing = false;

		ObjectiveComplete();
	}

	OnObjectiveUpdate.Broadcast(this, Progress);

}

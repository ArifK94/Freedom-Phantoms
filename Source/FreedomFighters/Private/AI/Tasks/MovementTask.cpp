// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/MovementTask.h"

UMovementTask::UMovementTask(const FObjectInitializer& ObjectInit)
{
}


void UMovementTask::ReceiveTickAI(AAIController* OwnerController, APawn* ControlledPawn, float DeltaSeconds)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ReceiveTickAI!"));

}


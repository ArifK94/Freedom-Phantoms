// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/UtilityAIAction.h"
#include "Characters/BaseCharacter.h"
#include "Controllers/CombatAIController.h"

UUtilityAIAction::UUtilityAIAction()
{
	bMarkedForDeath = false;
	bIsTimedOut = false;
	CooldownAmount = 5.f;
}

UWorld* UUtilityAIAction::GetWorld() const
{
	UObject* Outer = GetOuter();
	if (!Outer)
		return nullptr;
	AAIController* Controller = Cast<AAIController>(GetOuter());
	if (!Controller)
		return nullptr;

	return Controller->GetWorld();
}

void UUtilityAIAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	ReceiveTick(DeltaTime, Controller, Pawn);
}

bool UUtilityAIAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	return ReceiveCanRun(Controller, Pawn);
}

bool UUtilityAIAction::CanRunAsynchronously(AAIController* Controller, APawn* Pawn) const
{
	return false;
}

float UUtilityAIAction::Score(AAIController* Controller, APawn* Pawn)
{
	return ReceiveScore(Controller, Pawn);
}

void UUtilityAIAction::Enter(AAIController* Controller, APawn* Pawn)
{
	ReceiveEnter(Controller, Pawn);
}

void UUtilityAIAction::Exit(AAIController* Controller, APawn* Pawn)
{
	ReceiveExit(Controller, Pawn);
}

void UUtilityAIAction::Spawn(AAIController* Controller, APawn* Pawn)
{
	ReceiveSpawn(Controller, Pawn);

	CombatAIController = Cast<ACombatAIController>(Controller);
	OwningCombatCharacter = Cast<ACombatCharacter>(Pawn);
}

void UUtilityAIAction::Kill()
{
	bMarkedForDeath = true;
}

bool UUtilityAIAction::IsMarkedForDeath()
{
	return bMarkedForDeath;
}

bool UUtilityAIAction::IsTimedOut()
{
	return bIsTimedOut;
}

void UUtilityAIAction::Resurrect()
{
	bMarkedForDeath = false;
}

void UUtilityAIAction::StartTimeOut()
{
	GetWorld()->GetTimerManager().SetTimer(THandler_TimeOut, this, &UUtilityAIAction::StopTimeOut, 1.f, false, CooldownAmount);
	bIsTimedOut = true;
}

void UUtilityAIAction::StopTimeOut()
{
	bIsTimedOut = false;
	GetWorld()->GetTimerManager().ClearTimer(THandler_TimeOut);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/CoverAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/AIMovementComponent.h"
#include "CustomComponents/CoverFinderComponent.h"
#include "Services/SharedService.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

UCoverAction::UCoverAction()
{
	CooldownAmount = FMath::RandRange(8.f, 10.f);
}

float UCoverAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);
	return .8f;
}

bool UCoverAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	// if following commander then do not look for cover since the functionality is to be always near the commander.
	if (CombatAIController->GetCommander() && CombatAIController->GetCurrentCommand() == CommanderOrders::Follow) {
		return false;
	}


	// do not look for cover if using MG
	if (OwningCombatCharacter->GetMountedGun()) {
		return false;
	}

	// do not look for cover if in a vehicle
	if (OwningCombatCharacter->GetIsInVehicle()) {
		return false;
	}

	return true;
}

void UCoverAction::Exit(AAIController* Controller, APawn* Pawn)
{
	Super::Exit(Controller, Pawn);

	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_Peaking);
	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_EndPeaking);
	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_Stance);
}

void UCoverAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	mDeltaTime = DeltaTime;

	if (OwningCombatCharacter->IsTakingCover()) 
	{
		CombatAIController->SetIsRunningForCover(false);

		// is the enemy behind the NPC?
		// can the enemy see the cover point?
		// stop using this cover & search for another cover point.
		if (USharedService::IsTargetBehind(OwningCombatCharacter, CombatAIController->GetEnemyActor(), .0f) ||
			USharedService::CanSeeTarget(GetWorld(), CoverPoint.GetLocation(), CombatAIController->GetEnemyActor(), OwningCombatCharacter))
		{
			OwningCombatCharacter->StopCover();
			CombatAIController->SetCoverFound(false);

			OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_Peaking);
		}
		else 
		{
			if (!THandler_Peaking.IsValid()) 
			{
				OwningCombatCharacter->GetWorldTimerManager().SetTimer(THandler_Peaking, this, &UCoverAction::BeginCoverPeak, 1.f, true, FMath::RandRange(2.f, 5.f));
			}
		}
	}
	else if (!CombatAIController->GetCoverFound()) 
	{
		FindCover();
		MoveToResult = EPathFollowingRequestResult::Failed;
		CombatAIController->SetIsRunningForCover(true);
	}
	else if (CombatAIController->GetCoverFound())
	{
		if (CombatAIController->GetCoverFinderComponent()->IsCoverPointTaken(CoverPoint.GetLocation()))
		{
			CombatAIController->SetCoverFound(false);
		}
		else if (MoveToResult == EPathFollowingRequestResult::AlreadyAtGoal) 
		{
			TakeCover();
			CombatAIController->SetCoverFound(false);
			StartTimeOut();
		}
		else 
		{
			MoveToResult = CombatAIController->GetAIMovementComponent()->MoveToDestination(CoverPoint.GetLocation(), 10.f, AIBehaviourState::PriorityOrdersCommander);
			CombatAIController->SetCoverFound(true);
		}
	}

	// Change stance if no cover found & enemy is present.
	if (!CombatAIController->GetCoverFound() && CombatAIController->GetEnemyActor())
	{
		if (!THandler_Stance.IsValid())
		{
			OwningCombatCharacter->GetWorldTimerManager().SetTimer(THandler_Stance, this, &UCoverAction::ChangeStance, 1.f, true, FMath::RandRange(2.f, 5.f));
			StartTimeOut();
		}
	}
}

void UCoverAction::FindCover()
{
	// find a random cover point in range.
	bool HasCoverPoint = false;

	// find a cover point around enemy.
	if (CombatAIController->GetEnemyActor()) 
	{
		HasCoverPoint = CombatAIController->GetCoverFinderComponent()->FindCover(CombatAIController->GetEnemyActor(), CoverPoint);
	}
	else 
	{
		HasCoverPoint = CombatAIController->GetCoverFinderComponent()->FindCover(OwningCombatCharacter->GetActorLocation(), CoverPoint);
	}

	// is there a cover point to go to?
	if (HasCoverPoint) 
	{
		CombatAIController->SetTargetDestination(CoverPoint.GetLocation());
		CombatAIController->SetCoverFound(true);
	}

}

void UCoverAction::TakeCover()
{
	if (OwningCombatCharacter->IsTakingCover()) {
		return;
	}

	// disable combat until in cover.
	CombatAIController->SetDisableCombat(true);

	auto CharacterRot = OwningCombatCharacter->GetCapsuleComponent()->GetComponentRotation();

	auto TargetRot = UKismetMathLibrary::FindLookAtRotation(OwningCombatCharacter->GetCapsuleComponent()->GetComponentLocation(), CoverPoint.GetLocation());
	TargetRot.Roll = OwningCombatCharacter->GetActorRotation().Roll;
	TargetRot.Pitch = OwningCombatCharacter->GetActorRotation().Pitch;

	FRotator NewRot = UKismetMathLibrary::RLerp(CharacterRot, CoverPoint.GetRotation().Rotator(), mDeltaTime * 10.f, false);
	OwningCombatCharacter->GetCapsuleComponent()->SetWorldRotation(NewRot);

	OwningCombatCharacter->TakeCover();

	// AI can focus point on the enemy if taking cover.
	CombatAIController->SetDisableCombat(false);

}

void UCoverAction::BeginCoverPeak()
{
	// randomly choose whether to peak or aim from cover.
	bool ShouldAim = UKismetMathLibrary::RandomBool();

	if (CombatAIController->GetEnemyActor()) {
		ShouldAim = true;
	}

	if (OwningCombatCharacter->CanPerformCoverPeakUp()) 
	{

		if (ShouldAim)
		{
			if (!OwningCombatCharacter->IsAiming()) 
			{
				OwningCombatCharacter->BeginAim();
			}
		}
		else {
			OwningCombatCharacter->SetForwardInputValue(1.f);

			if (!THandler_EndPeaking.IsValid()) 
			{
				OwningCombatCharacter->GetWorldTimerManager().SetTimer(THandler_EndPeaking, this, &UCoverAction::EndCoverPeak, 1.f, true, FMath::RandRange(5.f, 10.f));
			}
		}
	}
}

void UCoverAction::EndCoverPeak()
{
	OwningCombatCharacter->SetForwardInputValue(.0f);
	OwningCombatCharacter->SetRightInputValue(.0f);

	OwningCombatCharacter->EndAim();

	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_Peaking);
	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_EndPeaking);
}

void UCoverAction::ChangeStance()
{
	if (OwningCombatCharacter->IsTakingCover() || CombatAIController->GetCoverFound()) {
		OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_Stance);
		return;
	}

	// toggle between crouch or stand based on a random bool value.
	if (UKismetMathLibrary::RandomBool()) {
		OwningCombatCharacter->ToggleCrouch();
	}

	OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_Stance);
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/CoverAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/AIMovementComponent.h"
#include "CustomComponents/CoverFinderComponent.h"
#include "Services/SharedService.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

float UCoverAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);
	return .8f;
}

bool UCoverAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	// if not near to target destination, then do not search for cover.
	//if (!SharedService::IsNearTargetPosition(OwningCombatCharacter->GetActorLocation(), CombatAIController->GetTargetDestination(), 10.f)) {
	//	return false;
	//}


	return true;
}

void UCoverAction::Spawn(AAIController* Controller, APawn* Pawn)
{
	Super::Spawn(Controller, Pawn);

	CombatAIController = Cast<ACombatAIController>(Controller);
	OwningCombatCharacter = Cast<ACombatCharacter>(Pawn);
}

void UCoverAction::Enter(AAIController* Controller, APawn* Pawn)
{
	Super::Enter(Controller, Pawn);
}

void UCoverAction::Exit(AAIController* Controller, APawn* Pawn)
{
	Super::Exit(Controller, Pawn);
}

void UCoverAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	if (OwningCombatCharacter->IsTakingCover()) {

		if (SharedService::IsTargetBehind(OwningCombatCharacter, CombatAIController->GetEnemyActor())) {
			OwningCombatCharacter->StopCover();
		}
		else {
			if (!THandler_Peaking.IsValid()) {
				OwningCombatCharacter->GetWorldTimerManager().SetTimer(THandler_Peaking, this, &UCoverAction::BeginCoverPeak, 1.f, true, FMath::RandRange(2.f, 5.f));
			}
		}
	}
	else if (!CoverFound) {
		FindCover();
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Searching for Cover!"));
	}
	else if (CoverFound) {

		if (CombatAIController->GetCoverFinderComponent()->IsCoverPointTaken(CoverLocation)) {
			CoverFound = false;
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Cover Taken!"));
		}
		else if (SharedService::IsNearTargetPosition(OwningCombatCharacter->GetActorLocation(), CoverLocation, 50.f)) {
			TakeCover();
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Take Cover!"));
		}
		else {
			CombatAIController->GetAIMovementComponent()->MoveToDestination(CoverLocation, .0f, AIBehaviourState::PriorityOrdersCommander);
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Moving to Cover!"));
		}

		DrawDebugSphere(GetWorld(), CoverLocation, 20.f, 1.f, FColor::Orange, false, 1.f, 0, 2);

	}
}

void UCoverAction::FindCover()
{
	auto TargetLocation = OwningCombatCharacter->GetActorLocation();

	//if (CombatAIController->GetEnemyActor()) {
	//	TargetLocation = CombatAIController->GetEnemyActor()->GetActorLocation();
	//}

	bool HasCoverPoint = CombatAIController->GetCoverFinderComponent()->FindCover(TargetLocation, CoverLocation);

	if (HasCoverPoint) {
		CombatAIController->SetTargetDestination(CoverLocation);
		CoverFound = true;
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Cover Found!"));
	}

}

void UCoverAction::TakeCover()
{
	if (!OwningCombatCharacter->IsTakingCover()) {
		OwningCombatCharacter->TakeCover();

		OwningCombatCharacter->GetWorldTimerManager().ClearTimer(THandler_Peaking);
	}
}

void UCoverAction::BeginCoverPeak()
{
	// randomly choose whether to peak or aim from cover.
	bool ShouldAim = UKismetMathLibrary::RandomBool();

	if (CombatAIController->GetEnemyActor()) {
		ShouldAim = true;
	}

	if (OwningCombatCharacter->CanCoverPeakUp()) {

		if (ShouldAim) {
			if (!OwningCombatCharacter->IsAiming()) {
				OwningCombatCharacter->BeginAim();
			}
		}
		else {
			OwningCombatCharacter->SetForwardInputValue(1.f);

			if (!THandler_EndPeaking.IsValid()) {
				OwningCombatCharacter->GetWorldTimerManager().SetTimer(THandler_EndPeaking, this, &UCoverAction::EndCoverPeak, 1.f, true, FMath::RandRange(2.f, 5.f));
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
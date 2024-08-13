// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/MountedGunAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "Weapons/MountedGun.h"
#include "Services/SharedService.h"

#include "CustomComponents/AIMovementComponent.h"
#include "CustomComponents/MountedGunFinderComponent.h"

float UMountedGunAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);

	return .82f;
}

bool UMountedGunAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	// If in patrol mode & no enemy in sight then do not search for MG
	if (CombatAIController->GetCurrentBehaviourState() == AIBehaviourState::Patrol) {
		if (!CombatAIController->GetEnemyActor()) {
			OwningCombatCharacter->DropMountedGun();
			return false;
		}
	}

	// if already using an MG
	// if using an aircraft MG for instance, which should not be exited, no need to run this action
	auto IsInVehicleMG = OwningCombatCharacter->GetMountedGun() && !OwningCombatCharacter->GetMountedGun()->GetCanExitMG();

	if (IsInVehicleMG) {
		return false;
	}

	if (OwningCombatCharacter->GetMountedGun()) {

		// Enemy is within MG range?
		if (!CombatAIController->GetMountedGunFinderComponent()->IsInTargetRange(OwningCombatCharacter->GetMountedGun(), OwningCombatCharacter, CombatAIController->GetEnemyActor())) {
			OwningCombatCharacter->DropMountedGun();
			return false;
		}

		// if there is an enemy, is the MG able to point at the enemy?
		if (CombatAIController->GetEnemyActor() && !CombatAIController->GetMountedGunFinderComponent()->IsInTargetRange(OwningCombatCharacter->GetMountedGun(), OwningCombatCharacter, CombatAIController->GetEnemyActor())) {
			OwningCombatCharacter->DropMountedGun();
			return false;
		}

		// Check if AI has a follow order, if so, then is the AI near the commander?
		// and MG is not near Commander.
		if (CombatAIController->GetCommander() && CombatAIController->GetCurrentCommand() == CommanderOrders::Follow && !CombatAIController->IsNearCommander(OwningCombatCharacter->GetMountedGun()->GetCharacterStandPos())) {
			OwningCombatCharacter->DropMountedGun();
			return false;
		}

		// in case player or another NPC has reached the MG before AI
		if (OwningCombatCharacter->GetMountedGun()->GetOwner() && OwningCombatCharacter->GetMountedGun()->GetOwner() != OwningCombatCharacter) {
			OwningCombatCharacter->DropMountedGun();
			return false;
		}

		// If near MG
		if (FVector::Distance(OwningCombatCharacter->GetActorLocation(), OwningCombatCharacter->GetMountedGun()->GetCharacterStandPos()) < 100.f)
		{
			// enemy is behind MG?
			if (USharedService::IsTargetBehind(OwningCombatCharacter->GetMountedGun(), CombatAIController->GetEnemyActor())) {
				OwningCombatCharacter->DropMountedGun();
				return false;
			}

			// is enemy behind me?
			if (USharedService::IsTargetBehind(OwningCombatCharacter, CombatAIController->GetEnemyActor())) {
				// Still posssess MG when enemy is no longer behing AI
				OwningCombatCharacter->DropMountedGun(false);
				return false;
			}
		}


	}
	else {
		return false;
	}

	if (OwningCombatCharacter->IsUsingMountedWeapon()) {
		return false;
	}

	return true;
}

void UMountedGunAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	CombatAIController->SetTargetDestination(OwningCombatCharacter->GetMountedGun()->GetCharacterStandPos());
	MoveToResult = CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetTargetDestination(), .0f, AIBehaviourState::Normal, true, false);

	MaintainMG();

}

void UMountedGunAction::FindMountedGun()
{
	auto EnemyActor = CombatAIController->GetEnemyActor();

	auto MountedGunFinderComponent = CombatAIController->GetMountedGunFinderComponent();

	auto SelectedMG = MountedGunFinderComponent->FindMG();

	// if found an MG 
	// & enemy is not behind the MG
	if (SelectedMG) {
		bool IsMGValid = true;

		if (EnemyActor && IsMGValid) {
			bool IsInRange = MountedGunFinderComponent->IsInTargetRange(SelectedMG, EnemyActor, OwningCombatCharacter);

			if (!IsInRange) {
				IsMGValid = false;
			}
			else if (USharedService::IsTargetBehind(SelectedMG, EnemyActor)) {
				IsMGValid = false;
			}
		}

		if (IsMGValid) {

			SelectedMG->SetPotentialOwner(OwningCombatCharacter);
			OwningCombatCharacter->SetMountedGun(SelectedMG);

			CombatAIController->SetTargetDestination(OwningCombatCharacter->GetMountedGun()->GetCharacterStandPos());
			CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetTargetDestination(), .0f, AIBehaviourState::Normal, true, false);
		}

	}

}

void UMountedGunAction::MaintainMG()
{
	auto EnemyActor = CombatAIController->GetEnemyActor();

	// If an MG has been assigned

	if (!OwningCombatCharacter->IsReloading()
		&& !OwningCombatCharacter->IsUsingMountedWeapon()
		&& !USharedService::IsTargetBehind(OwningCombatCharacter, EnemyActor)
		&& CombatAIController->GetMountedGunFinderComponent()->IsInTargetRange(OwningCombatCharacter->GetMountedGun(), OwningCombatCharacter, EnemyActor)) {
		// Don't want AI to teleport to the MG, needs to be close enough to use it
		auto DistanceToMG = FVector::Distance(OwningCombatCharacter->GetActorLocation(), OwningCombatCharacter->GetMountedGun()->GetCharacterStandPos());

		if (DistanceToMG < 100.f) {
			OwningCombatCharacter->UseMountedGun();
		}

		return;
	}
}
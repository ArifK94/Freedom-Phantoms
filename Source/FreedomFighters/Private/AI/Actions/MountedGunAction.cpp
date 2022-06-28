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

	if (OwningCombatCharacter->GetMountedGun() &&
		!OwningCombatCharacter->IsReloading() &&
		!OwningCombatCharacter->IsUsingMountedWeapon() &&
		!SharedService::IsTargetBehind(OwningCombatCharacter, CombatAIController->GetEnemyActor()) &&
		CombatAIController->GetMountedGunFinderComponent()->IsInTargetRange(OwningCombatCharacter->GetMountedGun(), OwningCombatCharacter, CombatAIController->GetEnemyActor())) {
		
		// Don't want AI to teleport to the MG, needs to be close enough to use it
		auto DistanceToMG = FVector::Distance(OwningCombatCharacter->GetActorLocation(), OwningCombatCharacter->GetMountedGun()->GetCharacterStandPos());

		if (DistanceToMG < 100.f) {
			OwningCombatCharacter->UseMountedGun();
		}

		return 1.f;
	}

	return .8f;
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

	return !IsInVehicleMG;
}

void UMountedGunAction::Spawn(AAIController* Controller, APawn* Pawn)
{
	Super::Spawn(Controller, Pawn);

	CombatAIController = Cast<ACombatAIController>(Controller);
	OwningCombatCharacter = Cast<ACombatCharacter>(Pawn);
}

void UMountedGunAction::Enter(AAIController* Controller, APawn* Pawn)
{
	Super::Enter(Controller, Pawn);
}

void UMountedGunAction::Exit(AAIController* Controller, APawn* Pawn)
{
	Super::Exit(Controller, Pawn);
}

void UMountedGunAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	if (OwningCombatCharacter->GetMountedGun()) {
		MaintainMG();
	}
	else {
		FindMountedGun();
	}
}

void UMountedGunAction::FindMountedGun()
{
	auto EnemyActor = CombatAIController->GetEnemyActor();

	bool CanFindMG = true;

	auto MountedGunFinderComponent = CombatAIController->GetMountedGunFinderComponent();

	if (!OwningCombatCharacter->GetMountedGun() && CanFindMG) {

		auto SelectedMG = MountedGunFinderComponent->FindMG();

		// if found an MG 
		// & enemy is not behind the MG
		if (SelectedMG) {
			bool IsMGValid = true;
			// Check if AI has a ollow order, if it's defend or attack then the if statement should be ignored
			// and commander is near the MG,
			if (CombatAIController->GetCommander() && 
				CombatAIController->GetCurrentCommand() == CommanderOrders::Follow && 
				!CombatAIController->IsNearCommander(SelectedMG->GetCharacterStandPos())) {
				IsMGValid = false;
			}

			if (EnemyActor && IsMGValid) {
				bool IsInRange = MountedGunFinderComponent->IsInTargetRange(SelectedMG, EnemyActor, OwningCombatCharacter);

				if (!IsInRange) {
					IsMGValid = false;
				}
				else if (SharedService::IsTargetBehind(SelectedMG, EnemyActor)) {
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
}

void UMountedGunAction::MaintainMG()
{
	auto EnemyActor = CombatAIController->GetEnemyActor();

	// If an MG has been assigned

	if (!OwningCombatCharacter->IsReloading()
		&& !OwningCombatCharacter->IsUsingMountedWeapon()
		&& !SharedService::IsTargetBehind(OwningCombatCharacter, EnemyActor)
		&& CombatAIController->GetMountedGunFinderComponent()->IsInTargetRange(OwningCombatCharacter->GetMountedGun(), OwningCombatCharacter, EnemyActor)) {
		// Don't want AI to teleport to the MG, needs to be close enough to use it
		auto DistanceToMG = FVector::Distance(OwningCombatCharacter->GetActorLocation(), OwningCombatCharacter->GetMountedGun()->GetCharacterStandPos());

		if (DistanceToMG < 100.f) {
			OwningCombatCharacter->UseMountedGun();
		}

		return;
	}

	// in case player or another NPC has reached the MG before AI
	if (OwningCombatCharacter->GetMountedGun()->GetOwner() != OwningCombatCharacter ||
		!CombatAIController->GetMountedGunFinderComponent()->IsInTargetRange(OwningCombatCharacter->GetMountedGun(), OwningCombatCharacter, EnemyActor)) {
		OwningCombatCharacter->DropMountedGun();
		return;
	}
	else if (SharedService::IsTargetBehind(OwningCombatCharacter, EnemyActor)) {
		// Still posssess MG when enemy is no longer behing AI
		OwningCombatCharacter->DropMountedGun(false);
		return;
	}

}
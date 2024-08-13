// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actions/RecruitAttackAction.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/AIMovementComponent.h"
#include "Services/SharedService.h"

#include "GameFramework/CharacterMovementComponent.h"

float URecruitAttackAction::Score(AAIController* Controller, APawn* Pawn)
{
	Super::Score(Controller, Pawn);
	return .9f;
}

bool URecruitAttackAction::CanRun(AAIController* Controller, APawn* Pawn) const
{
	Super::CanRun(Controller, Pawn);

	// does AI have a commander?
	if (CombatAIController->GetCommander() == nullptr) {
		return false;
	}

	// is the current order the FOLLOW command?
	if (CombatAIController->GetCurrentCommand() != CommanderOrders::Attack) {
		return false;
	}

	// 	No need to go up close to the HVT location, as long as the recruit NPC is within close distance & can see HVT, then this action does not need to run.
	if (CanSeeHVT()) {
		return false;
	}

	// is searching or in cover? To allow AI to find cover once it has its reached order destination.
	if (CombatAIController->GetIsRunningForCover() || OwningCombatCharacter->IsTakingCover()) {
		return false;
	}

	// if near destination to defend, then no need run this action any further.
	if (USharedService::IsNearTargetPosition(OwningCombatCharacter->GetActorLocation(), CombatAIController->GetRecruitInfo()->TargetLocation, 200.f)) {
		return false;
	}

	return true;
}

void URecruitAttackAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn)
{
	Super::Tick(DeltaTime, Controller, Pawn);

	// if high value target is assigned.
	if (CombatAIController->GetRecruitInfo()->HighValueTarget) {
		MoveToHVT();
	}
	else {
		MoveToAttackPosition();
	}
}

void URecruitAttackAction::MoveToAttackPosition()
{
	auto TargetDest = CombatAIController->GetRecruitInfo()->TargetLocation;
	CombatAIController->SetTargetDestination(TargetDest);
	CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetTargetDestination(), .0f, AIBehaviourState::PriorityOrdersCommander);
}

void URecruitAttackAction::MoveToHVT()
{
	auto TargetDest = CombatAIController->GetRecruitInfo()->TargetLocation;
	CombatAIController->SetTargetDestination(TargetDest);
	CombatAIController->GetAIMovementComponent()->MoveToDestination(CombatAIController->GetTargetDestination(), 10.f, AIBehaviourState::PriorityOrdersCommander);
}

bool URecruitAttackAction::CanSeeHVT() const
{
	auto TargetActor = CombatAIController->GetRecruitInfo()->HighValueTarget;

	if (TargetActor == nullptr) {
		return false;
	}

	auto Range = FMath::RandRange(800.f, 2000.f);
	
	// check if within close range to target. 
	auto IsNearTarget = USharedService::IsNearTargetPosition(OwningCombatCharacter->GetActorLocation(), TargetActor->GetActorLocation(), Range);

	if (!IsNearTarget) {
		return false;
	}

	// Start from NPC Head socket to target head socket.
	auto Socket = OwningCombatCharacter->GetHeadSocket();
	auto EyeLocation = OwningCombatCharacter->GetMesh()->GetSocketLocation(Socket);


	auto Socket2 = TargetActor->GetHeadSocket();
	auto HVTEyeLocation = TargetActor->GetMesh()->GetSocketLocation(Socket2);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwningCombatCharacter);
	QueryParams.bTraceComplex = false;


	FHitResult OutHit;
	auto IsHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		EyeLocation,
		HVTEyeLocation,
		ECC_Visibility,
		QueryParams
	);

	return !IsHit || (OutHit.GetActor() && OutHit.GetActor()->IsOwnedBy(TargetActor));
}

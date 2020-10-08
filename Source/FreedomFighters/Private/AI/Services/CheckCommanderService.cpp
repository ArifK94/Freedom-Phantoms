#include "AI/Services/CheckCommanderService.h"
#include "AIController.h"

#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"

#include "BehaviorTree/BlackboardComponent.h"


void UCheckCommanderService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	AAIController* AIOwner = OwnerComp.GetAIOwner(); // to get the controller
	APawn* Pawn = AIOwner->GetPawn(); // to get the actor

	ACombatCharacter* OwningCharacter = Cast<ACombatCharacter>(Pawn);

	ACommanderCharacter* Commander = OwningCharacter->getCommander();

	if (Commander != nullptr)
	{
		BlackboardComp->SetValueAsObject(BB_Commander.SelectedKeyName, Commander);
	}
}

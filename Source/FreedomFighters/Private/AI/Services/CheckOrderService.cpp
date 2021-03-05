#include "AI/Services/CheckOrderService.h"
#include "AIController.h"

#include "Characters/CombatCharacter.h"
#include "Characters/CommanderCharacter.h"

#include "BehaviorTree/BlackboardComponent.h"


void UCheckOrderService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	AAIController* AIOwner = OwnerComp.GetAIOwner(); // to get the controller
	APawn* Pawn = AIOwner->GetPawn(); // to get the actor

	ACombatCharacter* OwningCharacter = Cast<ACombatCharacter>(Pawn);

	ACommanderCharacter* Commander = OwningCharacter->getCommander();

	if (Commander != nullptr)
	{
		//UCommanderRecruit* Recruit = Commander->GetRecruitInfo(Pawn);

		//BlackboardComp->SetValueAsEnum(BB_OrderType.SelectedKeyName, (uint8)Recruit->CurrentCommand);
	}
}

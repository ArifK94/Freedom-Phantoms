#include "AI/Services/SelectEnemyService.h"
#include "Controllers/CombatAIController.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void USelectEnemyService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	AAIController* AIOwner = OwnerComp.GetAIOwner(); // to get the controller
	ACombatAIController* AICombatController = Cast<ACombatAIController>(AIOwner);


	if (AICombatController)
	{
		AICombatController->FindEnemy();

		AActor* SelectedTargetActor = AICombatController->GetEnemyActor();

		BlackboardComp->SetValueAsObject(BB_TargetActor.SelectedKeyName, SelectedTargetActor);
	}
}

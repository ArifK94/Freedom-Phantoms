// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/UtilityAIComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"

UUtilityAIComponent::UUtilityAIComponent()
{
	bCanRunWithoutPawn = true;
}

// Called when the game starts
void UUtilityAIComponent::BeginPlay()
{
	Super::BeginPlay();

	LastAction = nullptr;
	LastPawn = nullptr;

	LastSwitchTime = 0;

	// instantiate actions
	for (TSubclassOf<UUtilityAIAction> ActionClass : Actions)
	{
		SpawnActionInstance(ActionClass);
	}

	OnUtilityAIInitialized.Broadcast();
}

void UUtilityAIComponent::TimerTick()
{
	Super::TimerTick();

	if (!EnableUtilityAI) {
		return;
	}


	AAIController* Controller = Cast<AAIController>(GetOwner());
	if (!Controller) {
		return;
	}

	APawn* Pawn = Controller->GetPawn();

	if (!Pawn) {
		return;
	}


	OnUtilityAIBeforeScoreComputation.Broadcast();


	UUtilityAIAction* BestAction = nullptr;

	if (Pawn || bCanRunWithoutPawn)
	{
		BestAction = ComputeBestAction(Controller, Pawn);
	}

	for (UUtilityAIAction* Action : InstancedAsyncActions)
	{
		Action->LastCanRun = !Action->IsMarkedForDeath() && !Action->IsTimedOut() && Action->CanRun(Controller, Pawn) && Action->CanRunAsynchronously(Controller, Pawn);

		if (!Action->LastCanRun) {
			continue;
		}

		Action->Tick(WorldDeltaSeconds, Controller, Pawn);
		OnUtilityAIActionTicked.Broadcast(Action);
	}

	if (BestAction)
	{
		OnUtilityAIActionChoosen.Broadcast(BestAction);
		if (LastAction != BestAction)
		{
			float CurrentTime = GetWorld()->GetTimeSeconds();
			// avoid too fast action switching
			if (LastSwitchTime == 0 || CurrentTime - LastSwitchTime > Bounciness)
			{
				OnUtilityAIActionChanged.Broadcast(BestAction, LastAction);
				if (LastAction)
				{
					LastAction->Exit(Controller, LastPawn);
				}
				BestAction->Enter(Controller, Pawn);
				LastSwitchTime = CurrentTime;
			}
			else
			{
				// fast exit if nothing to run
				if (!LastAction)
					return;
				BestAction = LastAction;
			}
		}

#if WITH_EDITOR
		// Leave this commented out for debugging then uncomment it when needed.
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT
		// ("%s"), *BestAction->GetClass()->GetName()));
#endif

		BestAction->Tick(WorldDeltaSeconds, Controller, Pawn);
		LastAction = BestAction;
		LastPawn = Pawn;
		OnUtilityAIActionTicked.Broadcast(BestAction);
		return;
	}

	OnUtilityAIActionNotAvailable.Broadcast();

	if (LastAction)
	{
		OnUtilityAIActionChanged.Broadcast(nullptr, LastAction);
		LastAction->Exit(Controller, LastPawn);
		LastAction->Resurrect();
		LastAction = nullptr;
		LastPawn = nullptr;
	}
}

bool UUtilityAIComponent::CanSpawnActionInstance(TSubclassOf<UUtilityAIAction> ActionClass) const
{
	for (UUtilityAIAction* Action : InstancedActions)
	{
		if (Action->GetClass() == ActionClass)
			return false;
	}
	return true;
}

UUtilityAIAction* UUtilityAIComponent::SpawnActionInstance(TSubclassOf<UUtilityAIAction> ActionClass)
{
	// skip null
	if (!ActionClass)
		return nullptr;

	// avoid duplicates
	if (!CanSpawnActionInstance(ActionClass))
		return nullptr;

	AAIController* Controller = Cast<AAIController>(GetOwner());
	if (!Controller)
		return nullptr;

	UUtilityAIAction* Action = NewObject<UUtilityAIAction>(Controller, ActionClass);

	if (Action->CanRunAsynchronously(Controller, Controller->GetPawn()))
	{
		InstancedAsyncActions.Add(Action);
	}
	else
	{
		InstancedActions.Add(Action);
	}

	Action->Spawn(Controller, Controller->GetPawn());
	OnUtilityAIActionSpawned.Broadcast(Action);

	return Action;
}

bool UUtilityAIComponent::InternalRandBool() const
{
	float r = 0;
	if (bUseRandomStream)
		r = RandomStream.FRandRange(0.0f, 1.0f);
	else
		r = FMath::RandRange(0.0f, 1.0f);
	return 0.5f > r;
}

bool UUtilityAIComponent::CheckLowestScore(UUtilityAIAction* Current, UUtilityAIAction* Best) const
{
	// fast case
	if (!Best)
		return true;

	if (FMath::Abs(Best->LastScore - Current->LastScore) <= EqualityTolerance)
	{
		if (bInvertPriority)
			return true;
		if (bRandomizeOnEquality)
		{
			return InternalRandBool();
		}
		return false;
	}

	if (Best->LastScore > Current->LastScore)
	{
		return true;
	}


	return false;
}

bool UUtilityAIComponent::CheckHighestScore(UUtilityAIAction* Current, UUtilityAIAction* Best) const
{
	// fast case
	if (!Best)
		return true;

	if (FMath::Abs(Best->LastScore - Current->LastScore) <= EqualityTolerance)
	{
		if (bInvertPriority)
			return true;
		if (bRandomizeOnEquality)
		{
			return InternalRandBool();
		}
		return false;
	}

	if (Best->LastScore < Current->LastScore)
	{
		return true;
	}


	return false;
}

UUtilityAIAction* UUtilityAIComponent::ReceiveComputeBestAction_Implementation(AAIController* Controller, APawn* Pawn)
{
	UUtilityAIAction* BestAction = nullptr;

	for (UUtilityAIAction* Action : InstancedActions)
	{
		// do not compute asynchronous actions as they need to be run at the same time with another action.
		Action->LastCanRun = !Action->IsMarkedForDeath() && Action->CanRun(Controller, Pawn) && !Action->CanRunAsynchronously(Controller, Pawn);
		if (!Action->LastCanRun)
			continue;
		Action->LastScore = ScoreFilter(Action, Action->Score(Controller, Pawn));
		if (bIgnoreZeroScore && Action->LastScore == 0)
			continue;

		if (bUseLowestScore)
		{
			if (CheckLowestScore(Action, BestAction))
				BestAction = Action;
		}
		else
		{
			if (CheckHighestScore(Action, BestAction))
				BestAction = Action;
		}
	}

	return BestAction;
}

TArray<UUtilityAIAction*> UUtilityAIComponent::GetActionInstances() const
{
	return InstancedActions.Array();
}

UUtilityAIAction* UUtilityAIComponent::GetActionInstanceByClass(TSubclassOf<UUtilityAIAction> ActionClass) const
{
	for (UUtilityAIAction* Action : InstancedActions)
	{
		if (Action->GetClass() == ActionClass)
			return Action;
	}
	return nullptr;
}

UUtilityAIAction* UUtilityAIComponent::GetCurrentActionInstance() const
{
	return LastAction;
}

TSubclassOf<UUtilityAIAction> UUtilityAIComponent::GetCurrentActionClass() const
{
	return LastAction ? LastAction->GetClass() : nullptr;
}

void UUtilityAIComponent::SetRandomStream(FRandomStream InRandomStream)
{
	RandomStream = InRandomStream;
	bUseRandomStream = true;
}

FRandomStream UUtilityAIComponent::GetRandomStream() const
{
	return RandomStream;
}

UUtilityAIAction* UUtilityAIComponent::ComputeBestAction(AAIController* Controller, APawn* Pawn)
{
	return ReceiveComputeBestAction(Controller, Pawn);
}

void UUtilityAIComponent::SetEnableUtilityAI(bool Enabled)
{
	EnableUtilityAI = Enabled;
	PrimaryComponentTick.bCanEverTick = Enabled;

	// if not enabled, then do not tick
	if (!Enabled) {

		if (LastAction) 
		{
			LastAction->Kill();
			LastAction->Exit(nullptr, nullptr);
			LastAction = nullptr;
		}
	}
}
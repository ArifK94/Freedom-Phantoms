// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyActorComponent.generated.h"

/**
* Contains custom common functionality for actor components
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UMyActorComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	UPROPERTY()
		class ABaseCharacter* OwningCharacter;
	/**
	* Owning Pawn / Actor.
	*/
	UPROPERTY()
		AActor* OwningPawn;

	UPROPERTY()
		AController* OwningController;

	UPROPERTY()
		FTimerHandle THandler_TimerTick;

	UPROPERTY()
		FTimerHandle THandler_BeginInit;

	UPROPERTY()
		FTimerHandle THandler_StopInit;

	// has the init function be initialized?
	bool HasInit;

protected:
	float WorldDeltaSeconds;

public:	
	UMyActorComponent();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
		virtual void TimerTick();

	UFUNCTION(BlueprintCallable)
		virtual void Init();

private:
	void BeginTick();

	void BeginInit();

	void StopInitTimer();
		
public:
	/**
	* OverrideTimer will stop timer if timer is currently running.
	*/
	void StartTickTimer(bool OverrideTimer = false);


	/**
	* Add specific rate of interval for the time
	* OverrideTimer will stop timer if timer is currently running.
	*/
	void StartTickTimer(float InRate, bool OverrideTimer = false);

	void StopTickTimer();

public:
	ABaseCharacter* GetOwningCharacter() { return OwningCharacter; }

	AActor* GetOwningPawn() { return OwningPawn; }

	AController* GetController() { return OwningController; }

};

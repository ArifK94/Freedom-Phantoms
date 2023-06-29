// Fill out your copyright notice in the Description page of Project Settings.
/**
* AI Defending strongholds, all behaviours related to defending stronghold.
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StructCollection.h"
#include "StrongholdDefenderComponent.generated.h"

class AStronghold;
class UCoverPointComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDefenderPointFoundSignature, FStrongholdDefenderParams, StrongholdDefenderParams);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UStrongholdDefenderComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	FTimerHandle THandler_StrongholdCoverPoint;

	UPROPERTY()
		AStronghold* Stronghold;

	// to take defensive positions within the stronghold
	UPROPERTY()
		UCoverPointComponent* ChosenCoverPointComponent;

public:	
	UStrongholdDefenderComponent();

	static void SetDefender(AActor* Defender, AStronghold* Stronghold);

	void FindDefenderPoint();

	/** Remove defender from stronghold & make stronghold null */
	UFUNCTION(BlueprintCallable)
		void RemoveStronghold();

	void ClearTimer();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnDefenderPointFoundSignature OnDefenderPointFound;

public:
	AStronghold* GetStronghold() { return Stronghold; }

	void SetStronghold(AStronghold* Target) { Stronghold = Target; }

	UCoverPointComponent* GetChosenCoverPointComponent() { return ChosenCoverPointComponent; }


};

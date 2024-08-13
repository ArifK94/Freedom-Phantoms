// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnumCollection.h"
#include "TeamFactionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UTeamFactionComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TeamFaction SelectedFaction;

	/** Used to determine whether this component can be included during searching for team factions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsCompActive;


public:	
	UTeamFactionComponent();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		static bool IsFriendly(AActor* AActorA, AActor* ActorB);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		static bool IsComponentActive(AActor* Owner);

public:	
	TeamFaction GetSelectedFaction() { return SelectedFaction; }

	bool GetIsCompActive() { return IsCompActive; }

	void SetCompActive(bool Value) { IsCompActive = Value; }
		
};

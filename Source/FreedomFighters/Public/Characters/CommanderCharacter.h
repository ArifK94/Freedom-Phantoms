// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CombatCharacter.h"
#include "CommanderCharacter.generated.h"

class ACombatCharacter;

UENUM(BlueprintType)
enum class CommanderOrders : uint8
{
	Attack		UMETA(DisplayName = "Attack"),
	Defend 		UMETA(DisplayName = "Defend"),
	Follow		UMETA(DisplayName = "Follow")
};

UCLASS()
class FREEDOMFIGHTERS_API ACommanderCharacter : public ACombatCharacter
{
	GENERATED_BODY()

public:
	ACommanderCharacter();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operative Commands", meta = (AllowPrivateAccess = "true"))
		CommanderOrders CurrentCommand;



	TArray<ACombatCharacter*> CommanderFollowers;

	void CheckRecruit();

	void Recruit(ACombatCharacter* Character);


protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	
};

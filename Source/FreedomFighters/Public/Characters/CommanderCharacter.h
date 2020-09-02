// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CombatCharacter.h"

#include "Engine/DataTable.h"


#include "CommanderCharacter.generated.h"

class ACombatCharacter;

UENUM(BlueprintType)
enum class CommanderOrders : uint8
{
	Attack		UMETA(DisplayName = "Attack"),
	Defend 		UMETA(DisplayName = "Defend"),
	Follow		UMETA(DisplayName = "Follow")
};

USTRUCT(BlueprintType)
struct FCommanderFollower : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		ACombatCharacter* Follower;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		CommanderOrders CurrentCommand;
};


UCLASS()
class FREEDOMFIGHTERS_API ACommanderCharacter : public ACombatCharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		TArray<FCommanderFollower> ActorFollowers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		ACombatCharacter* CurrentCombatCharacter;

public:
	ACommanderCharacter();

private:
	void CheckRecruit();

	void Recruit();

	bool IfAlreadyRecruited(AActor* TargetActor);

	void ResetTargetActor();


protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	
};

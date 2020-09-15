// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CombatCharacter.h"

#include "Engine/DataTable.h"

#include "CommanderCharacter.generated.h"

class ACombatCharacter;
class AOrderIcon;

UENUM(BlueprintType)
enum class CommanderOrders : uint8
{
	Attack		UMETA(DisplayName = "Attack"),
	Defend 		UMETA(DisplayName = "Defend"),
	Follow		UMETA(DisplayName = "Follow")
};

USTRUCT(BlueprintType)
struct FCommanderRecruit : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		ACombatCharacter* Recruit;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		CommanderOrders CurrentCommand;
};


UCLASS()
class FREEDOMFIGHTERS_API ACommanderCharacter : public ACombatCharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		TArray<FCommanderRecruit> ActiveRecruits;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		FCommanderRecruit CurrentRecruit;

		ACombatCharacter* LastRecruit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		ACombatCharacter* PotentialRecruit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		uint8 CurrentRecruitIndex;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		FVector TargetDefendLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> OrderIcon;
	AOrderIcon* OrderIconObj;

	TArray<AOrderIcon*> OrderIconArray;

public:
	ACommanderCharacter();

private:

	FHitResult GetCurrentTraceHit(float Length = 500.0f);

	void CheckRecruit();

	void Recruit();

	void RecruitPlaySound();

	UFUNCTION(BlueprintCallable, Category = "Commander")
	bool IfAlreadyRecruited(AActor* TargetActor);

	void ResetTargetActor();

	UFUNCTION(BlueprintCallable, Category = "Commander")
		void OnAudioFinished();


	UFUNCTION(BlueprintCallable, Category = "Commander")
	FCommanderRecruit GetRecruitInfo(AActor* TargetActor);

	void DefendArea();

	void SpawnIcon();

	bool HasOrderIcon();

	void IncrementCurrentRecruit();

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CombatCharacter.h"

#include "Engine/DataTable.h"

#include "CommanderCharacter.generated.h"

class ABaseCharacter;
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
		ABaseCharacter* HighValueTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		CommanderOrders CurrentCommand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives")
		FVector TargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives")
		FTimerHandle THandler_ResponseSound;

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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		uint8 MaxRecruits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Order Icon", meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* AttackMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Order Icon", meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* DefendMaterial;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> OrderIcon;
	AOrderIcon* OrderIconObj;

	TArray<AOrderIcon*> OrderIconArray;



public:
	ACommanderCharacter();

	UFUNCTION(BlueprintCallable, Category = "Commander")
		FCommanderRecruit GetRecruitInfo(AActor* TargetActor);

private:

	FHitResult GetCurrentTraceHit(float Length = 500.0f);

	void CheckRecruit();

	void Recruit();


	UFUNCTION(BlueprintCallable, Category = "Commander")
		bool IfAlreadyRecruited(AActor* TargetActor);

	void ResetTargetActor();

	void Attack();

	void DefendArea();

	void FollowCommander();

	void SpawnIcon(UMaterialInterface* Material);

	bool HasOrderIcon();

	void IncrementCurrentRecruit();

	void PlayVoiceSound(USoundBase* SoundBase, FCommanderRecruit TargetRecruit);

	void PlayAcknowledgeSound(FCommanderRecruit TargetRecruit);


protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
	FCommanderRecruit GetCurrentRecruit() {
		return  CurrentRecruit;
	}
};

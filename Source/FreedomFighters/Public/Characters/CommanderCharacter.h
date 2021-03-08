// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CombatCharacter.h"

#include "Engine/DataTable.h"

#include "CommanderCharacter.generated.h"

class ABaseCharacter;
class ACombatCharacter;
class AOrderIcon;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemoveRecruitSignature, ACommanderCharacter*, Commander, int, Index);

UENUM(BlueprintType)
enum class CommanderOrders : uint8
{
	Attack		UMETA(DisplayName = "Attack"),
	Defend 		UMETA(DisplayName = "Defend"),
	Follow		UMETA(DisplayName = "Follow")
};

UCLASS(Blueprintable)
class FREEDOMFIGHTERS_API UCommanderRecruit : public UObject
{
	GENERATED_BODY()

protected:
	virtual bool IsSupportedForNetworking() const override { return true; };

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		ACombatCharacter* Recruit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		ABaseCharacter* HighValueTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		CommanderOrders CurrentCommand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector TargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FTimerHandle THandler_ResponseSound;


	// the array will contain all the order icons so only one icon is displayed at a time
	// the arrays will be seperated between overhead and posotion icons as the expected result can have both types of icons displaying at the same time
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<AOrderIcon*> OrderIconArray;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<AOrderIcon*> OverheadIconArray;


	AOrderIcon* AttackPositionIcon;
	AOrderIcon* HighValueTargetOverheadIcon; // this icon of HVT acts like a position icon overhead of an enemy
	AOrderIcon* DefendPositionIcon;

	AOrderIcon* AttackOverheadIcon;
	AOrderIcon* DefendOverheadIcon;
	AOrderIcon* FollowOverheadIcon;
};

UCLASS()
class FREEDOMFIGHTERS_API ACommanderCharacter : public ACombatCharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		TArray<UCommanderRecruit*> ActiveRecruits;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		UCommanderRecruit* CurrentRecruit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		ACombatCharacter* PotentialRecruit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		uint8 CurrentRecruitIndex;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		uint8 MaxRecruits;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> CommanderHUDWidgetClass;
	UUserWidget* CommanderHUDWidget;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> AttackPositionIconClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> HighValueTargetOverheadClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> DefendIconPositionClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> AttackOverheadClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> DefendOverheadClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> FollowOverheadClass;


public:
	ACommanderCharacter();

	UFUNCTION(BlueprintCallable, Category = "Commander")
		UCommanderRecruit* GetRecruitInfo(AActor* TargetActor);

	void AddUIWidget();

private:

	FHitResult GetCurrentTraceHit(float Length = 500.0f);

	void CheckRecruit();

	void Recruit();


	UFUNCTION(BlueprintCallable, Category = "Commander")
		bool IfAlreadyRecruited(AActor* TargetActor);

	UPROPERTY(BlueprintAssignable, Category = "Commander")
		FOnRemoveRecruitSignature OnRemoveRecruit;

	void ResetTargetActor();

	void Attack();

	void DefendArea();

	void FollowCommander();

	void UpdateActiveRecruits();

	// Each time a recruit dies, the array needs to be sorted by shifting all elements to previous empty elements
	void SortActiveRecruits(int StartingPoint);

	// modify target position so it is placed on the nav bounds
	FNavLocation GetPositionToNav(FVector Position);

	void SpawnIcon(TSubclassOf<AOrderIcon> IconClass, AOrderIcon*& Icon);
	void DisplayPositionIcon(AOrderIcon* SelectedIcon, TArray<AOrderIcon*> Icons, FVector Location);
	void DisplayOverheadIcon(AOrderIcon* SelectedIcon, TArray<AOrderIcon*> Icons);
	void HideAllIcons(TArray<AOrderIcon*> Icons);

	void IncrementCurrentRecruit();

	void PlayVoiceSound(USoundBase* SoundBase, UCommanderRecruit* TargetRecruit);

	void PlayAcknowledgeSound(UCommanderRecruit* TargetRecruit);

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
};

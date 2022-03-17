#pragma once

#include "CoreMinimal.h"
#include "Characters/CombatCharacter.h"
#include "Interfaces/Interactable.h"
#include "EnumCollection.h"
#include "Engine/DataTable.h"

#include "CommanderCharacter.generated.h"

class ABaseCharacter;
class ACombatCharacter;
class AOrderIcon;
class UUserWidget;
class UTargetFinderComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemoveRecruitSignature, ACommanderCharacter*, Commander, int, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOrderSentSignature, UCommanderRecruit*, RecruitInfo);


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
class FREEDOMFIGHTERS_API ACommanderCharacter : public ACombatCharacter, public IInteractable
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTargetFinderComponent* TargetSeekerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		TArray<UCommanderRecruit*> ActiveRecruits;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		UCommanderRecruit* CurrentRecruit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		ACombatCharacter* PotentialRecruit;

	/** Used to prevent recruiting while in HQ or other places */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		bool CanRecruit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		uint8 CurrentRecruitIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		uint8 MaxRecruits;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		FName RecruitMessage;

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
		UMaterialInstance* AttackIconMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		UMaterialInstance* DefendIconMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		UMaterialInstance* FollowIconMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		UMaterialInstance* AttackShapeMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		UMaterialInstance* DefendShapeMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		UMaterialInstance* FollowShapeMaterial;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> AttackOverheadClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> DefendOverheadClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> FollowOverheadClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		int OperativeKillCounter;

public:
	ACommanderCharacter();

	UFUNCTION(BlueprintCallable, Category = "Commander")
		UCommanderRecruit* GetRecruitInfo(AActor* TargetActor);

	FOnOrderSentSignature OnOrderSent;

	void AddUIWidget();

	void Recruit();

	void Attack(bool CommandAll = false);

	void DefendArea(bool CommandAll = false);

	void FollowCommander(bool CommandAll = false);

private:

	UFUNCTION()
		void OnOperativeKillConfirm(int KillCount);

	FHitResult GetCurrentTraceHit(float Length = 500.0f);

	void CheckRecruit();


	void AttackSingle(UCommanderRecruit* Recruit, ABaseCharacter* EnemyCharacter, FHitResult HitResult);

	void DefendAreaSingle(UCommanderRecruit* Recruit);

	void FollowSingle(UCommanderRecruit* Recruit);


	UFUNCTION(BlueprintCallable, Category = "Commander")
		bool IfAlreadyRecruited(AActor* TargetActor);

	UPROPERTY(BlueprintAssignable, Category = "Commander")
		FOnRemoveRecruitSignature OnRemoveRecruit;

	void ResetTargetActor();

	void UpdateActiveRecruits();

	// Each time a recruit dies, the array needs to be sorted by shifting all elements to previous empty elements
	void SortActiveRecruits(int StartingPoint);

	// modify target position so it is placed on the nav bounds
	FNavLocation GetPositionToNav(FVector Position);

	void SpawnIcon(TSubclassOf<AOrderIcon> IconClass, AOrderIcon*& Icon);
	void DisplayPositionIcon(AOrderIcon* SelectedIcon, TArray<AOrderIcon*> Icons, FVector Location);
	void HideAllIcons(TArray<AOrderIcon*> Icons);

	void IncrementCurrentRecruit();

	void PlayCommunicationSound(USoundBase* SoundBase, UCommanderRecruit* TargetRecruit);

	void PlayAcknowledgeSound(UCommanderRecruit* TargetRecruit);

protected:
	virtual void Tick(float DeltaTime) override;


public:
	// Interactable interface methods
	virtual FString GetKeyDisplayName_Implementation() override;
	virtual FString OnInteractionFound_Implementation(APawn* InPawn, AController* InController) override;
	virtual AActor* OnPickup_Implementation(APawn* InPawn, AController* InController) override;
	virtual bool OnUseInteraction_Implementation(APawn* InPawn, AController* InController) override;
	virtual bool CanInteract_Implementation(APawn* InPawn, AController* InController) override;

public:
	void SetCanRecruit(bool Value) { CanRecruit = Value; }
};

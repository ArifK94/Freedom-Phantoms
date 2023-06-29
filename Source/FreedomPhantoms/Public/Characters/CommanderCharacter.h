#pragma once

#include "CoreMinimal.h"
#include "Characters/CombatCharacter.h"
#include "EnumCollection.h"
#include "Engine/DataTable.h"

#include "CommanderCharacter.generated.h"

class ABaseCharacter;
class ACombatCharacter;
class AOrderIcon;
class UUserWidget;
class UTargetFinderComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemoveRecruitSignature, ACommanderCharacter*, Commander, int, RecruitIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOrderSentSignature, UCommanderRecruit*, RecruitInfo, int, RecruitIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRecruitHealthChangeSignature, UCommanderRecruit*, RecruitInfo, int, RecruitIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCommanderChangeSignature, ACommanderCharacter*, NewCommander);


UCLASS(Blueprintable)
class FREEDOMPHANTOMS_API UCommanderRecruit : public UObject
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

	/** Handler delay when an order is carried out. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FTimerHandle THandler_OrderDelay;


	// the array will contain all the order icons so only one icon is displayed at a time
	// the arrays will be seperated between overhead and posotion icons as the expected result can have both types of icons displaying at the same time
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<AOrderIcon*> OrderIconArray;

	UPROPERTY()
		AOrderIcon* AttackPositionIcon;

	/** this icon of HVT acts like a position icon overhead of an enemy */
	UPROPERTY()
		AOrderIcon* HighValueTargetOverheadIcon;

	UPROPERTY()
		AOrderIcon* DefendPositionIcon;
};

UCLASS()
class FREEDOMPHANTOMS_API ACommanderCharacter : public ACombatCharacter
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTargetFinderComponent* TargetSeekerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		TArray<UCommanderRecruit*> ActiveRecruits;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		int WoundedCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		UCommanderRecruit* CurrentRecruit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		ACombatCharacter* PotentialRecruit;

	/** Used to prevent recruiting while in HQ or other places */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		bool CanSearchRecruits;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		uint8 CurrentRecruitIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		uint8 MaxRecruits;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		FName RecruitMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		FName ReviveMessage;
	FName CurrentMessage;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> AttackPositionIconClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> DefendIconPositionClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Orders", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AOrderIcon> HighValueTargetOverheadClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Operatives", meta = (AllowPrivateAccess = "true"))
		int OperativeKillCounter;

public:
	ACommanderCharacter();

	UFUNCTION(BlueprintCallable, Category = "Commander")
		UCommanderRecruit* GetRecruitInfo(AActor* TargetActor);

	UPROPERTY(BlueprintAssignable, Category = "Commander")
		FOnOrderSentSignature OnOrderSent;

	UPROPERTY(BlueprintAssignable, Category = "Commander")
		FOnRecruitHealthChangeSignature OnRecruitHealthChange;

	UPROPERTY(BlueprintAssignable, Category = "Commander")
		FOnCommanderChangeSignature OnCommanderChange;

	void InteractWithOperative();

	void Attack(bool CommandAll = false);

	void DefendArea(bool CommandAll = false);

	void FollowCommander(bool CommandAll = false);

	void CheckRecruit();

	void ChangeCommander(ACommanderCharacter* NewCommander);


private:

	UFUNCTION()
		void OnOperativeKillConfirm(int KillCount);

	UFUNCTION(BlueprintCallable)
		void OnRecruitHealthUpdate(FHealthParameters InHealthParameters);

	UPROPERTY(BlueprintAssignable, Category = "Commander")
		FOnRemoveRecruitSignature OnRemoveRecruit;

	FHitResult GetCurrentTraceHit(float Length = 500.0f);

	void RecruitFollower();

	void ReviveFriendly();

	void RemoveWounded();

	void AttackSingle(UCommanderRecruit* Recruit, ABaseCharacter* EnemyCharacter, FHitResult HitResult, float Delay = 0.f);

	void DefendAreaSingle(UCommanderRecruit* Recruit, float Delay = 0.f);

	void FollowSingle(UCommanderRecruit* Recruit, float Delay = 0.f);

	void OrderRecruit(UCommanderRecruit* RecruitInfo, float Delay = 0.f);

	void BroadcastOrderDelay(UCommanderRecruit* RecruitInfo);

	void ResetTargetActor();

	void UpdateActiveRecruits();

	// Each time a recruit dies, the array needs to be sorted by shifting all elements to previous empty elements
	void SortActiveRecruits(int StartingPoint, bool RemoveIndex);

	/* Sort the list with the wounded recruit being positioned the last */
	void SortRecruitList();

	// modify target position so it is placed on the nav bounds
	FNavLocation GetPositionToNav(FVector Position);

	void SpawnIcon(TSubclassOf<AOrderIcon> IconClass, AOrderIcon*& Icon);
	void DisplayPositionIcon(AOrderIcon* SelectedIcon, TArray<AOrderIcon*> Icons, FVector Location, bool CountdownHideIcon = true);
	void DisplayPositionIcon(AOrderIcon* SelectedIcon, TArray<AOrderIcon*> Icons, bool CountdownHideIcon = true);
	void HideAllIcons(TArray<AOrderIcon*> Icons);

	void IncrementCurrentRecruit();

	void PlayCommunicationSound(USoundBase* SoundBase, UCommanderRecruit* TargetRecruit);

	void PlayAcknowledgeSound(UCommanderRecruit* TargetRecruit);

protected:
	virtual void Tick(float DeltaTime) override;

public:
	void SetCanSearchRecruits(bool Value) { CanSearchRecruits = Value; }

	void SetActiveRecruits(TArray<UCommanderRecruit*> Recruits) { ActiveRecruits = Recruits; }

	void SetCurrentRecruit(UCommanderRecruit* Recruit) { CurrentRecruit = Recruit; }

	void SetCurrentRecruitIndex(uint8 Index) { CurrentRecruitIndex = Index; }

	void SetWoundedCount(int Count) { WoundedCount = Count; }


	bool GetCanSearchRecruits() { return CanSearchRecruits; }

	bool GetCanRecruit();

	UCommanderRecruit* GetCurrentRecruit() { return CurrentRecruit; }

	ACombatCharacter* GetPotentialRecruit() { return PotentialRecruit; }

	FName GetRecruitMessage() { return RecruitMessage; }

	FName GetCurrentMessage() { return CurrentMessage; }

};

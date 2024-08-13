// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomComponents/Engine/MyActorComponent.h"
#include "StructCollection.h"
#include "Interfaces/Chatable.h"
#include "BattleChatterComponent.generated.h"


class AMountedGun;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UBattleChatterComponent : public UMyActorComponent, public IChatable
{
	GENERATED_BODY()

private:

	UPROPERTY()
		class ACombatCharacter* CombatCharacter;

	UPROPERTY()
		class ACombatAIController* CombatAIController;

	FVoiceClipSet* VoiceClipsSet;

	UPROPERTY()
		TArray<FBattleChatterParams> BattleChatters;

	UPROPERTY()
		FTimerHandle THandler_DelaySound;

		/** Keep a reference of the last sound when invoking GetBattleChatterSound() to avoid duplicate invoke after a second.  */
		USoundBase* PreviousBattleSoundInserted;

public:	
	UBattleChatterComponent();

	void PlayTargetFound();

	void PlaySuppressingFire();

	void PlayMoveCombat();

	void PlayCoverMe();

	void PlayStayAlert();

private:
	UFUNCTION()
		void OnTargetSearchUpdate(FTargetSearchParameters TargetSearchParameters);

	UFUNCTION()
		void OnCoverSearchUpdate(FCoverSearchParameters CoverSearchParameters);

	UFUNCTION()
		void OnMountedGunEnabledUpdate(AMountedGun* MountedGun);

	UFUNCTION()
		void OnHealthUpdate(FHealthParameters InHealthParameters);

	UFUNCTION()
		void OnWeaponUpdate(FWeaponUpdateParameters WeaponUpdateParameters);

	virtual void OnCallerReceived_Implementation(FChatableParams ChatableParams) override;

	void PlayDelaySound(USoundBase* Sound);

	bool CanPlay(int max = 3);

	void AddBattleChatterItem();

	USoundBase* GetBattleChatterSound(USoundBase* Sound);

	void CooldownChatter(FBattleChatterParams ChatterParam);

	void SendReponseCall(ACombatCharacter* Character, FChatableParams ChatableParams);

	ACombatCharacter* FindNearChatableFriendly();

protected:
	virtual void BeginPlay() override;

	virtual void TimerTick() override;


		
};

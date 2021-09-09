// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameStateBaseCustom.generated.h"

class UAudioComponent;
class ABaseObjective;
class USoundBase;
UCLASS()
class FREEDOMFIGHTERS_API AGameStateBaseCustom : public AGameStateBase
{
	GENERATED_BODY()

private:
	/** Assigned when the music is changed */
	float MusicStateTarget;

	/** To lerp to music target state */
	float CurrentMusicState;

	int TotalObjectives;
	float TotalObjectiveFactor;
	TArray<ABaseObjective*> Objectives;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* MusicAudioComponent;

	/** Defined in Sound Cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName MusicStateParamName;

	/** Defined in Sound Cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float MusicChangeInterpolation;

	bool HasGameEnded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsMissionPassed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundBase* MissionPassedMusic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundBase* MissionFailedMusic;


public:
	AGameStateBaseCustom();

	void EndGame(bool MissionPassed);

private:
	UFUNCTION()
		void OnObjectiveUpdate(ABaseObjective* Objective, float Progress);

	void CalculateTotalProgression();

protected:
	virtual void BeginPlay() override;

private:
	virtual void Tick(float DeltaTime) override;

public:
	TArray<ABaseObjective*> GetObjectives() {
		return Objectives;
	}

	bool GetHasGameEnded() {
		return HasGameEnded;
	}
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "StructCollection.h"
#include "GameStateBaseCustom.generated.h"

class UAudioComponent;
class ABaseObjective;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameEndedSignature, bool, isMissionPassed);

UCLASS()
class FREEDOMPHANTOMS_API AGameStateBaseCustom : public AGameStateBase
{
	GENERATED_BODY()

private:
	/** Assigned when the music is changed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float MusicStateTarget;

	/** To lerp to music target state */
	float CurrentMusicState;

	int TotalObjectives;
	float TotalObjectiveFactor;
	TArray<ABaseObjective*> Objectives;

	bool HasGameEnded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* MusicAudioComponent;

	/** Defined in Sound Cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName MusicStateParamName;

	/** Defined in Sound Cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float MusicChangeInterpolation;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsMissionPassed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundBase* MissionPassedMusic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundBase* MissionFailedMusic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundBase* NearEndMusic;

	UPROPERTY()
		TArray<FObjectPoolParameters> ProjectilesPool;

	/** Contains list of music sounds to be played after another. */
	UPROPERTY()
		TArray<USoundBase*> MusicQueueList;
	FTimerHandle THandler_MusicQueue;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		UDataTable* SurfaceImpactSetDatatable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		UDataTable* SurfaceImpactDatatable;


public:
	AGameStateBaseCustom();

	FSurfaceImpactSet* RetrieveSurfaceImpactSet(FName RowName);

	FSurfaceImpact* RetrieveSurfaceImpact(FName RowName);


	void EndGame(bool MissionPassed);

	UFUNCTION(BlueprintCallable)
		void PlayMusic(USoundBase* Music);

	UFUNCTION(BlueprintCallable)
		void PlayMusicInQueue();

	UFUNCTION(BlueprintCallable)
		void AddMusicToQueue(USoundBase* Music);

	UFUNCTION(BlueprintCallable)
		void ClearMusicQueue();

	UFUNCTION(BlueprintCallable)
		void ContinueMusic();

	UFUNCTION(BlueprintCallable)
		void StopMusic();

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnGameEndedSignature OnGameEnded;

	AObjectPoolActor* GetAvailablePoolActor(TSubclassOf<AActor> ActorClass);

	void AddPoolActor(FObjectPoolParameters PoolableActor);

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

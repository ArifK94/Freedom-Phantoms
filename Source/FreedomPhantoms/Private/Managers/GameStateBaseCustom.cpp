// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/GameStateBaseCustom.h"
#include "Managers/ObjectiveManager.h"
#include "Objectives/BaseObjective.h"
#include "ObjectPoolActor.h"

#include "Components/AudioComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AGameStateBaseCustom::AGameStateBaseCustom()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));

	MusicChangeInterpolation = 5.0f;
	MusicStateParamName = "State";

	CanPlayerRespawn = false;

	HasGameEnded = false;
	IsMissionPassed = false;
}

void AGameStateBaseCustom::BeginPlay()
{
	Super::BeginPlay();

	ObjectiveManager = NewObject<UObjectiveManager>(this);
	ObjectiveManager->OnMissionCompleted.AddDynamic(this, &AGameStateBaseCustom::OnMissionCompleted);

	LoadObjectives();
}

void AGameStateBaseCustom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Smooth transition when changing music
	CurrentMusicState = FMath::FInterpTo(CurrentMusicState, MusicStateTarget, DeltaTime, MusicChangeInterpolation);
	MusicAudioComponent->SetFloatParameter(MusicStateParamName, CurrentMusicState);
}

void AGameStateBaseCustom::OnObjectiveUpdate(ABaseObjective* Objective, float Progress)
{
	float totalProgress = 0.0f;

	// calculate the average of the total progress
	for (int i = 0; i < ObjectiveManager->GetObjectives().Num(); i++)
	{
		// only update progress of compulsory objectives.
		if (!ObjectiveManager->GetObjectives()[i]->GetIsOptional())
		{
			totalProgress += ObjectiveManager->GetObjectives()[i]->GetProgress();
		}
	}

	// update music as near to completing level.
	if (totalProgress > 0)
	{
		// update the music state
		MusicStateTarget = totalProgress / ObjectiveManager->GetTotalRequiredObjectives();

		PlayMusic(NearEndMusic);
	}
}

void AGameStateBaseCustom::OnMissionCompleted(bool HasCompleted)
{
	EndGame(true);
}

void AGameStateBaseCustom::LoadObjectives()
{
	ObjectiveManager->LoadObjectives();

	for (int i = 0; i < ObjectiveManager->GetTotalObjectives(); i++)
	{
		ABaseObjective* Objective = Cast<ABaseObjective>(ObjectiveManager->GetObjectives()[i]);

		Objective->OnObjectiveUpdate.AddDynamic(this, &AGameStateBaseCustom::OnObjectiveUpdate);
	}
}


void AGameStateBaseCustom::EndGame(bool MissionPassed)
{
	// prevent iteration if already ended.
	if (HasGameEnded) {
		return;
	}

	ClearMusicQueue();

	HasGameEnded = true;
	IsMissionPassed = MissionPassed;

	if (MissionPassed)
	{
		if (MissionPassedMusic)
		{
			MusicAudioComponent->Sound = MissionPassedMusic;
			MusicAudioComponent->Play();
		}

	}
	else
	{
		if (MissionFailedMusic)
		{
			MusicAudioComponent->Sound = MissionFailedMusic;
			MusicAudioComponent->Play();
		}
	}

	OnGameEnded.Broadcast(MissionPassed);
}

void AGameStateBaseCustom::PlayMusic(USoundBase* Music)
{
	if (!Music || HasGameEnded || MusicAudioComponent->GetSound() == Music) {
		return;
	}

	MusicAudioComponent->Sound = Music;
	MusicAudioComponent->Play();
}

void AGameStateBaseCustom::PlayMusicInQueue()
{
	// clear timer if no music in queue.
	if (MusicQueueList.Num() <= 0) {
		GetWorldTimerManager().ClearTimer(THandler_MusicQueue);
	}

	USoundBase* Music = MusicQueueList[0];

	MusicAudioComponent->Sound = Music;
	MusicAudioComponent->Play();

	MusicQueueList.RemoveAt(0);

	GetWorldTimerManager().SetTimer(THandler_MusicQueue, this, &AGameStateBaseCustom::PlayMusicInQueue, Music->Duration, false);
}

void AGameStateBaseCustom::AddMusicToQueue(USoundBase* Music)
{
	if (!Music) {
		return;
	}

	MusicQueueList.Add(Music);
}

void AGameStateBaseCustom::ClearMusicQueue()
{
	GetWorldTimerManager().ClearTimer(THandler_MusicQueue);
	MusicQueueList.Empty();
}

void AGameStateBaseCustom::ContinueMusic()
{
	if (MusicAudioComponent->GetSound() == nullptr) {
		return;
	}

	MusicAudioComponent->Play();
}

void AGameStateBaseCustom::StopMusic()
{
	MusicAudioComponent->Stop();
}

AObjectPoolActor* AGameStateBaseCustom::GetAvailablePoolActor(TSubclassOf<AActor> ActorClass)
{
	for (int i = 0; i < ProjectilesPool.Num(); i++)
	{
		auto PoolParam = ProjectilesPool[i];

		if (!UKismetSystemLibrary::IsValid(PoolParam.PoolableActor)) {
			continue;
		}


		if (PoolParam.PoolableActorClass == ActorClass && !PoolParam.PoolableActor->IsActive())
		{
			return PoolParam.PoolableActor;
		}
	}

	return nullptr;
}

void AGameStateBaseCustom::AddPoolActor(FObjectPoolParameters PoolableActor)
{
	ProjectilesPool.Add(PoolableActor);
}
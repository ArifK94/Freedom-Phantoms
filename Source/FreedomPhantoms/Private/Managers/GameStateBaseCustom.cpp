// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/GameStateBaseCustom.h"
#include "Objectives/BaseObjective.h"
#include "ObjectPoolActor.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

AGameStateBaseCustom::AGameStateBaseCustom()
{
	PrimaryActorTick.bCanEverTick = true;

	MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));

	MusicChangeInterpolation = 5.0f;
	MusicStateParamName = "State";

	TotalObjectives = 0;

	HasGameEnded = false;
	IsMissionPassed = false;
}

FSurfaceImpactSet* AGameStateBaseCustom::RetrieveSurfaceImpactSet(FName RowName)
{
	if (SurfaceImpactSetDatatable == nullptr) {
		return nullptr;
	}

	static const FString ContextString(TEXT("Surface Impact Set DataSet"));
	return SurfaceImpactSetDatatable->FindRow<FSurfaceImpactSet>(RowName, ContextString, true);
}

FSurfaceImpact* AGameStateBaseCustom::RetrieveSurfaceImpact(FName RowName)
{
	if (SurfaceImpactDatatable == nullptr) {
		return nullptr;
	}

	static const FString ContextString(TEXT("Surface Impact DataSet"));
	return SurfaceImpactDatatable->FindRow<FSurfaceImpact>(RowName, ContextString, true);
}

void AGameStateBaseCustom::BeginPlay()
{
	Super::BeginPlay();

	CalculateTotalProgression();
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
	for (int i = 0; i < Objectives.Num(); i++)
	{
		// only update progress of compulsory objectives.
		if (!Objectives[i]->GetIsOptional())
		{
			totalProgress += Objectives[i]->GetProgress();
		}
	}

	// update music as near close to completing objectives.
	if (totalProgress > 0)
	{
		// update the music state
		MusicStateTarget = totalProgress / TotalObjectives;

		PlayMusic(NearEndMusic);
	}

}

void AGameStateBaseCustom::CalculateTotalProgression()
{
	TArray<AActor*> ObjectiveActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseObjective::StaticClass(), ObjectiveActors);

	TotalObjectives = ObjectiveActors.Num();
	// get the total factor as the max music state will be at 1.0f
	TotalObjectiveFactor = 1.0f / TotalObjectives;

	for (int i = 0; i < TotalObjectives; i++)
	{
		ABaseObjective* Objective = Cast<ABaseObjective>(ObjectiveActors[i]);

		Objective->OnObjectiveUpdate.AddDynamic(this, &AGameStateBaseCustom::OnObjectiveUpdate);

		Objectives.Add(Objective);
	}
}

void AGameStateBaseCustom::EndGame(bool MissionPassed)
{
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
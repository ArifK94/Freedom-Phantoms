// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CustomSaveGame.generated.h"

/**
 *
 */
UCLASS()
class FREEDOMPHANTOMS_API UCustomSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UCustomSaveGame();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		float MasterVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		float SFXVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		float VoiceVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		float MusicVolume;

public:
	float GetMasterVolume() {
		return MasterVolume;
	}

	float GetSFXVolume() {
		return SFXVolume;
	}

	float GetVoiceVolume() {
		return VoiceVolume;
	}

	float GetMusicVolume() {
		return MusicVolume;
	}


};

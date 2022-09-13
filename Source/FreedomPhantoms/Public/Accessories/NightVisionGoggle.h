// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightVisionGoggle.generated.h"

class USkeletalMeshComponent;
class UAudioComponent;
class UUserWidget;
class AGameHUDController;
UCLASS()
class FREEDOMPHANTOMS_API ANightVisionGoggle : public AActor
{
	GENERATED_BODY()

public:
	ANightVisionGoggle();

	void SetVisorState();

	void AddNVGWidget();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UPostProcessComponent* VisionPPComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isVisorOn;

	AGameHUDController* gameHUDController;

	FTimerHandle UnusedHandle;


	UAudioComponent* NVGAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* NVGOnSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* NVGOffSound;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> NVGWidgetClass;
	UUserWidget* NVGWidget;


	bool canToggle;

public:

	UFUNCTION(BlueprintCallable, Category = "NVG")
		void ToggleVision();

	bool getVisorState() { return isVisorOn; }

	USkeletalMeshComponent* GetMesh() {
		return Mesh;
	}

protected:

	virtual void BeginPlay() override;
};

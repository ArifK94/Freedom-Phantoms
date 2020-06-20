// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightVisionGoggle.generated.h"

class USkeletalMeshComponent;
class UAudioComponent;
UCLASS()
class FREEDOMFIGHTERS_API ANightVisionGoggle : public AActor
{
	GENERATED_BODY()
	
public:	
	ANightVisionGoggle();

	void SetVisorState();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Night Vision Goggles", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Night Vision Goggles", meta = (AllowPrivateAccess = "true"))
	class UPostProcessComponent* VisionPPComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Night Vision Goggles", meta = (AllowPrivateAccess = "true"))
	bool isVisorOn;

	class AGameHUDController* gameHUDController;

	FTimerHandle UnusedHandle;


	 UAudioComponent* NVGAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision Goggles Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* NVGOnSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision Goggles Sounds", meta = (AllowPrivateAccess = "true"))
		USoundBase* NVGOffSound;


	bool canToggle;

public:	

	UFUNCTION(BlueprintCallable, Category = "NVG")
	void ToggleVision();

	bool getVisorState() { return isVisorOn;  }

protected:

	virtual void BeginPlay() override;
};

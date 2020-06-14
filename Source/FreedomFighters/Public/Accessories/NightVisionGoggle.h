// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightVisionGoggle.generated.h"

class USkeletalMeshComponent;
UCLASS()
class FREEDOMFIGHTERS_API ANightVisionGoggle : public AActor
{
	GENERATED_BODY()
	
public:	
	ANightVisionGoggle();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Night Vision Goggles", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Night Vision Goggles", meta = (AllowPrivateAccess = "true"))
	class UPostProcessComponent* VisionPPComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision Goggles", meta = (AllowPrivateAccess = "true"))
	bool isVisorOn;

public:	

	void ToggleVision();

	bool getVisorState() { return isVisorOn;  }

protected:

	virtual void BeginPlay() override;
};

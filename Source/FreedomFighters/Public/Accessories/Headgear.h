// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Accessories/Accessory.h"

#include "Headgear.generated.h"

class USpotLightComponent;
UCLASS()
class FREEDOMFIGHTERS_API AHeadgear : public AAccessory
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Headgear", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Headgear Attachments", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class AGoggle> Goggle;
	 AGoggle* GoggleObj;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Headgear Attachments", meta = (AllowPrivateAccess = "true"))
		FName GoggleSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Headgear Attachments", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class ANightVisionGoggle> NightVisionGoggle;
	ANightVisionGoggle* NightVisionGoggleObj;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Headgear Attachments", meta = (AllowPrivateAccess = "true"))
		FName NVGSocket;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Headgear", meta = (AllowPrivateAccess = "true"))
		bool IsGoggleOff;

public:	
	AHeadgear();


	void ToggleRandomAccessory();

	void ToggleGoggles();

	void SpawnGoggle();
	void SpawnNVG();

	ANightVisionGoggle* getNightVision() {
		return NightVisionGoggleObj;
	}


protected:
	virtual void BeginPlay() override;
};

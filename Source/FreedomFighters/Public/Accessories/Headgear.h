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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory Helmet", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Nightvision_Holder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory Helmet", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Nightvision_Goggles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory Helmet", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Goggles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory Helmet", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Torchlight_Holder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory Helmet", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Torchlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory Helmet", meta = (AllowPrivateAccess = "true"))
		USpotLightComponent* TorchBeam;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory Helmet", meta = (AllowPrivateAccess = "true"))
		USpotLightComponent* LaserBeam;

private:

	bool isNightVisionOn;
	bool isGogglesOff;
	
public:	
	AHeadgear();


	void ToggleRandomAccessory();

	void ToggleVisor();

	void ToggleGoggles();


};

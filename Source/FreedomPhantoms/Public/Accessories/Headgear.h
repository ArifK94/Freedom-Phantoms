// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Accessories/Accessory.h"

#include "Headgear.generated.h"

class USpotLightComponent;
UCLASS()
class FREEDOMPHANTOMS_API AHeadgear : public AAccessory
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachments", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class AGoggle> GoggleClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments", meta = (AllowPrivateAccess = "true"))
		AGoggle* GoggleObj;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachments", meta = (AllowPrivateAccess = "true"))
		FName GoggleSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachments", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class ANightVisionGoggle> NightVisionGoggleClass;
	ANightVisionGoggle* NightVisionGoggleObj;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachments", meta = (AllowPrivateAccess = "true"))
		FName NVGSocket;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsGoggleOn;

public:	
	AHeadgear();

	void ToggleRandomAccessory();

	void ToggleGoggles();

	void SpawnGoggle();
	void SpawnNVG();

	ANightVisionGoggle* getNightVision() {
		return NightVisionGoggleObj;
	}

	FName GetParentSocket() {
		return ParentSocket;
	}

private:
	void EnableActor(AActor* Actor, bool IsEnabled);

protected:
	virtual void BeginPlay() override;
};

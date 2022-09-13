// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponAttachment.generated.h"

class USkeletalMeshComponent;

UCLASS()
class FREEDOMPHANTOMS_API AWeaponAttachment : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName ParentSocket;

	/** The main body material index for the attachment which will be used to change the material based on the weapon camo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int32 BodyMaterialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* DesertCamoMaterial;
	
public:	
	AWeaponAttachment();

	UFUNCTION(BlueprintCallable)
		void SetDesertCamo();

	FName GetParentSocket() { return ParentSocket; }

};

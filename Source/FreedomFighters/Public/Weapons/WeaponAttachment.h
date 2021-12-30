// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponAttachment.generated.h"

class USkeletalMeshComponent;

UCLASS()
class FREEDOMFIGHTERS_API AWeaponAttachment : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName ParentSocket;
	
public:	
	AWeaponAttachment();


	FName GetParentSocket() { return ParentSocket; }


};

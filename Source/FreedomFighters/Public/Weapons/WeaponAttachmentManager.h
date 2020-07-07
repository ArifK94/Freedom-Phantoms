// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponAttachmentManager.generated.h"

class AWeapon;
class AWeaponOptic;
class AWeaponLaser;
class AWeaponTorchlight;

UCLASS(Blueprintable)
class FREEDOMFIGHTERS_API UWeaponAttachmentManager : public UObject
{
	GENERATED_BODY()
	
private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attachment", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<AWeapon>> UnderBarrelWeaponClasses;
	 AWeapon* UnderBarrelWeaponObj;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attachment", meta = (AllowPrivateAccess = "true"))
	//	TSubclassOf<class AActor> UnderBarrelClasses;
	//class AActor* UnderBarrelObj;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attachment", meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<AWeaponOptic>> OpticClasses;
	 AWeaponOptic* OpticObj;


	 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attachment", meta = (AllowPrivateAccess = "true"))
		 TArray<TSubclassOf<AWeaponLaser>> LaserClasses;
	 AWeaponLaser* LaserObj;

	 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attachment", meta = (AllowPrivateAccess = "true"))
		 TArray<TSubclassOf<AWeaponTorchlight>> TorchlightClasses;
	 AWeaponTorchlight* TorchlightObj;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attachment", meta = (AllowPrivateAccess = "true"))
	//	TSubclassOf<class AActor> AccessoryClasses;
	//class AActor* AccessoryObj;
	
	UWorld* World;

public:	
	UWeaponAttachmentManager();

	UFUNCTION(BlueprintCallable, Category = "My Functions")
		void SpawnAttachments(USkeletalMeshComponent* mesh, AWeapon* owner, UWorld* ActorWorld);

	void SpawnUnderBarrel(USkeletalMeshComponent* mesh, AWeapon* owner);
	void SpawnOptics(USkeletalMeshComponent* mesh, AWeapon* owner);
	void SpawnLaser(USkeletalMeshComponent* mesh, AWeapon* owner);
	void SpawnTorch(USkeletalMeshComponent* mesh, AWeapon* owner);

	TArray<TSubclassOf<AWeapon>> getUnderBarrelWeaponClasses() {
		return UnderBarrelWeaponClasses;
	}

	AWeapon* getUnderBarrelWeaponObj() { return UnderBarrelWeaponObj; }

protected:
	virtual bool IsSupportedForNetworking() const override { return true; };


};

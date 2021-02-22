// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponClip.generated.h"

class AWeaponBullet;
class USoundAttenuation;

UCLASS()
class FREEDOMFIGHTERS_API AWeaponClip : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponClip();

	void DropClip(USkeletalMeshComponent* MeshComp, FName ClipSocket, TSubclassOf<class AWeaponClip> weaponClip);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* clipMeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		int ammoCapacity;

	int CurrentAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundBase* HighImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* ClipAttenuationSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Bullet", meta = (AllowPrivateAccess = "true"))
		TSubclassOf< AWeaponBullet> WeaponBulletClass;
	 AWeaponBullet* BulletObj;

	AWeaponClip* DroppedClip;

private:

	UFUNCTION(BlueprintCallable, Category = "Collision")
	void OnClipHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SpawnBullet();

protected:
	virtual void BeginPlay() override;


public:
	virtual void Tick(float DeltaTime) override;

	UStaticMeshComponent* getClipMesh() { return clipMeshComp; }
	int GetAmmoCapacity() const { return ammoCapacity; }

	int GetCurrentAmmo() const { return CurrentAmmo; }
	void SetCurrentAmmo(int value);

	AWeaponBullet* getBulletObj() { 
		return BulletObj; 
	}

	TSubclassOf<AWeaponBullet> getBulletClass() {
		return WeaponBulletClass;
	}

};

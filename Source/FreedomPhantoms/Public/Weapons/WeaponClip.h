// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponClip.generated.h"

class AProjectile;
class USoundAttenuation;

UCLASS()
class FREEDOMPHANTOMS_API AWeaponClip : public AActor
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* CollisionAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundBase* HighImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* ClipAttenuationSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Bullet", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AProjectile> WeaponBulletClass;
	AProjectile* BulletObj;

	AWeaponClip* DroppedClip;

private:

	UFUNCTION(BlueprintCallable, Category = "Collision")
	void OnClipHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SpawnBullet();

protected:
	virtual void BeginPlay() override;


public:
	UStaticMeshComponent* getClipMesh() { return clipMeshComp; }
	int GetAmmoCapacity() const { return ammoCapacity; }

	int GetCurrentAmmo() const { return CurrentAmmo; }
	void SetCurrentAmmo(int value);

	AProjectile* getBulletObj() {
		return BulletObj; 
	}

	TSubclassOf<AProjectile> getBulletClass() {
		return WeaponBulletClass;
	}

};

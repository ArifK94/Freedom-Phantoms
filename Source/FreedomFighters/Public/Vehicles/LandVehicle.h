// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LandVehicle.generated.h"

class USkeletalMeshComponent;
class USkeletalMesh;
class UHealthComponent;
class URadialForceComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class FREEDOMFIGHTERS_API ALandVehicle : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UHealthComponent* HealthComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		URadialForceComponent* RadialForceComp;

	/** Impulse applied to mesh when it explosed to boost up a little  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float ExplosionImpulse;

	/** Explosion damage applied to nearby health components */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float ExplosionDamage;

	/** Particle to play when health reaches zero  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ExplosionEffect;
	
	/** Sound to play when health reaches zero  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* ExplosionSound;

	/** Explosion sound attentuation  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* ExplosionAttenuation;

	/** The mesh to replace the original mesh once exploded  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMesh* ExplosionMesh;

	/** Enabled physics during explosion  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool SimulateExplosionPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsDestroyed;

	int KillCount;
	
public:	
	ALandVehicle();

	UFUNCTION(BlueprintCallable)
		void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo);

protected:
	virtual void BeginPlay() override;

private:	
	void ApplyExplosionDamage(FVector ImpactPoint, AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet);
	//void AddKill(UHealthComponent* DamagedActorHealth, UHealthComponent* OwnerHealth);
};

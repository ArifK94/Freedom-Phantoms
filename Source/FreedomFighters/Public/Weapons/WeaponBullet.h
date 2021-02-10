#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBullet.generated.h"

class UArrowComponent;
class USphereComponent;
class UParticleSystem;
class USoundBase;
class UDamageType;
class UProjectileMovementComponent;

UCLASS()
class FREEDOMFIGHTERS_API AWeaponBullet : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBullet();

	void Explode();

private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet", meta = (AllowPrivateAccess = "true"))
		USphereComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet", meta = (AllowPrivateAccess = "true"))
		UProjectileMovementComponent* BulletMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet Effects", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ExplosionParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet Effects", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet Effects", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Effects", meta = (AllowPrivateAccess = "true"))
		USoundBase* ImpactSound;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet", meta = (AllowPrivateAccess = "true"))
		bool isAnExplosive;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Damage", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Damage", meta = (AllowPrivateAccess = "true"))
		float DamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Damage", meta = (AllowPrivateAccess = "true"))
		float DamageRadius;

	FTimerHandle THandler_TimeBetweenShots;


private:
	UFUNCTION()
		void OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UParticleSystem* CheckSurface(EPhysicalSurface SurfaceType);

	void SetDestructableHit(UPrimitiveComponent* OtherComp);

protected:
	virtual void BeginPlay() override;

public:	

	float getDamageAmount() {
		return DamageAmount;
	}

	UStaticMeshComponent* getMesh() {
		return Mesh;
	}
};

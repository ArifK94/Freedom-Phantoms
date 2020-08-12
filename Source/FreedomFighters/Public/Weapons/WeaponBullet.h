// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBullet.generated.h"

class UCapsuleComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class FREEDOMFIGHTERS_API AWeaponBullet : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBullet();

	void Explode();

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet", meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Effects", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Effects", meta = (AllowPrivateAccess = "true"))
		USoundBase* ImpactSound;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet", meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* BulletMovement;

	FTimerHandle THandler_TimeBetweenShots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Damage", meta = (AllowPrivateAccess = "true"))
		float DamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Damage", meta = (AllowPrivateAccess = "true"))
		float DamageRadius;


	UFUNCTION(BlueprintCallable, Category = "Collision")
		void OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;


	float getDamageAmount() {
		return DamageAmount;
	}

	UStaticMeshComponent* getMesh()
	{
		return Mesh;
	}
};

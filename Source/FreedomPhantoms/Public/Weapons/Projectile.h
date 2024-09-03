#pragma once

#include "CoreMinimal.h"
#include "ObjectPoolActor.h"
#include "StructCollection.h"
#include "Projectile.generated.h"

class UParticleSystem;
class USoundBase;
class UDamageType;
class AWeapon;
class UHealthComponent;
class UTeamFactionComponent;
class UAudioComponent;
class UArrowComponent;
class USphereComponent;
class ACombatCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileImpactSignature, FProjectileImpactParameters, ProjectileImpactParameters);
UCLASS()
class FREEDOMPHANTOMS_API AProjectile : public AObjectPoolActor
{
	GENERATED_BODY()

private:

	/** The detection sphere to check actors within the projectile radius */
	UPROPERTY()
		USphereComponent* DetectionSphere;

	float CurrentDeltaTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USphereComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* BulletMovementAudio;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* CollisionAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		UDataTable* SurfaceImpactDatatable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		FName RowName;
	FSurfaceImpact* SurfaceImpact;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* ImpactAttenuation;

	/** Attentuation for the collision sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* CollisionAttenuation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
		USoundBase* TravelSound;

	/** The audio asset to play when projectile collides with something */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
		USoundBase* CollisionSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
		bool isAnExplosive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
		float DamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
		float ExplosiveRadiusInner;

	/** The outer radius which would affect world objects but will not affect health components, should always be higher than inner explosive radius */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
		float ExplosiveRadiusOuter;

	/** Radial impulse strengh */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
		float RadialForceStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (AllowPrivateAccess = "true"))
		bool IgnoreOwner;

	/** Can this projectile be affected by other projectiles? If other explosives detonate while this projectile is within their radius, can this projectile be exploded? Only applies to explosives. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (AllowPrivateAccess = "true"))
		bool IgnoreDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		bool ShowDebug;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		float DebugExplosionLifeTime;


	/** Rotate towards the eye points camera direction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Homing", meta = (AllowPrivateAccess = "true"))
		bool UseLaserGuidance;

	/** Homing projectile to lerp to target direction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Homing", meta = (AllowPrivateAccess = "true"))
		float HomingFollowFactor;

	// Custom Projectile Movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
		bool UseCustomProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
		float InitialSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
		float Mass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
		float Drag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
		FVector Gravity;

	/** Perform Hit detection. Only works if using custom movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
		bool DetectProjectileHit;

	/** Spin projectile while there is velocity? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
		bool SpinOnVelocity;

	/** Detect nearby overlapping actors when velocity is near zero. Used for grenade avoidance for example. Radius detection based on ExplosiveRadiusOuter amount.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
		bool DetectNearbyActors;

	FVector Velocity;
	FVector Acceleration;
	FVector NextPosition;
	FVector PreviousPosition;

	/** Destroy projectile rather than deactivate, in case it is not used as an object pool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pooling", meta = (AllowPrivateAccess = "true"))
		bool DestroyOnDeactivate;

	/** Countdown for the projectile to be destroyed, useful for grenades (Less than zero means there will be no countdown timer) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
		float CountdownTimer;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
		float DecalSizeMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
		float DecalSizeMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
		float DecalRotationMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
		float DecalRotationMax;

	/** Destroy decal component after time runs out (0 = infinite) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
		float DecalLifetime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
		float DecalFadeOutDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class UCameraShakeBase> CameraShake;

	/** Cameras inside this radius are ignored */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
		float CamShakeInnerRadius;

	/** Cameras outside of InnerRadius and inside this are effected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
		float CamShakeOuterRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AWeapon* WeaponParent;

	int KillCount;

	UPROPERTY()
		UHealthComponent* OwnerHealth;

	UPROPERTY()
		UTeamFactionComponent* OwnerFaction;

	UPROPERTY()
		ACombatCharacter* OwningCombatCharacter;

	FTimerHandle THandler_CountdownTimer;

	FHitResult LastHit;

	FRotator CurrentRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AActor* HomingTargetActor;


public:
	AProjectile();

	virtual void Activate() override;

	virtual void Deactivate() override;

	void SelfDestruct();

	UPROPERTY(BlueprintAssignable)
		FOnProjectileImpactSignature OnProjectileImpact;

	void SetCountdownTimer(float Value);

	/** Perform a missle target. */
	void FindHomingTarget(AActor* TargetActor);

private:
	void Init();

	void RetrieveSurfaceImpactSet();

	void Movement();

	/** Rotate projectile while moving */
	void SpinOnMovement();

	/** Perform laser guidance to follow the eye point. */
	void LaserGuidance();

	void DetectHit();

	void Explode(FVector ImpactPoint);


	bool IsHittingTarget(AActor* TargetActor, FVector ImpactPoint, float Radius);

	FSurfaceImpactSet CheckSurface(EPhysicalSurface SurfaceType);

	void AddKill(UHealthComponent* DamagedActorHealth, UTeamFactionComponent* DamagedActorFaction);

	bool IsInAir();

	FCollisionQueryParams GetQueryParams();

	void SetVFX(FSurfaceImpactSet ImpactSurface, FVector ImpactLocation);

	/** Check overalpping using the DetectionSphere */
	void AlertNearbyActors();

	UFUNCTION()
		void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable)
		void OnImpactHit(FHitResult InHit);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;


	/** Play sound when collided with something. */
	UFUNCTION(BlueprintCallable)
		void PlayCollisionSound(FVector Position);

public:
	float getDamageAmount() {
		return DamageAmount;
	}

	UStaticMeshComponent* getMesh() {
		return Mesh;
	}

	void SetWeaponParent(AWeapon* Weapon) {
		WeaponParent = Weapon;
	}

	bool IsExplosive() {
		return isAnExplosive;
	}

	bool GetIgnoreDamage() { return IgnoreDamage; }
};

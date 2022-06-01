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
class FREEDOMFIGHTERS_API AProjectile : public AObjectPoolActor
{
	GENERATED_BODY()

public:
	AProjectile();

	virtual void Activate() override;

	virtual void Deactivate() override;

	UPROPERTY(BlueprintAssignable)
		FOnProjectileImpactSignature OnProjectileImpact;

private:

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
		float ExplosiveRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (AllowPrivateAccess = "true"))
		bool IgnoreOwner;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		bool ShowExplosionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		float DebugExplosionLifeTime;


	/** Homing projectile to follow the direction of the weapon's direction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Homing", meta = (AllowPrivateAccess = "true"))
		bool HomingFollowWeaponEyePoint;

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

	FVector Velocity;
	FVector Acceleration;
	FVector NextPosition;
	FVector PreviousPosition;

	/** Destroy projectile rather than deactivate, in case it is not used as an object pool */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Pooling", meta = (AllowPrivateAccess = "true"))
		bool DestroyOnDeactivate;

	/** Countdown for the projectile to be destroyed, useful for grenades (Less than zero means there will be no countdown timer) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
		float CountdownTimer;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal", meta = (AllowPrivateAccess = "true"))
		float DecalSizeMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal", meta = (AllowPrivateAccess = "true"))
		float DecalSizeMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal", meta = (AllowPrivateAccess = "true"))
		float DecalRotationMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal", meta = (AllowPrivateAccess = "true"))
		float DecalRotationMax;

	/** Destroy decal component after time runs out (0 = infinite) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal", meta = (AllowPrivateAccess = "true"))
		float DecalLifetime;

	AWeapon* WeaponParent;

	int KillCount;

	UHealthComponent* OwnerHealth;
	UTeamFactionComponent* OwnerFaction;
	ACombatCharacter* OwningCombatCharacter;

	FTimerHandle THandler_CountdownTimer;

	FHitResult LastHit;


private:
	void Init();

	void RetrieveSurfaceImpactSet();

	void Movement();

	void FollowEyePoint();

	void DetectHit();

	void Explode(FVector ImpactPoint);

	void SelfDestruct();

	FSurfaceImpactSet CheckSurface(EPhysicalSurface SurfaceType);

	void AddKill(UHealthComponent* DamagedActorHealth, UTeamFactionComponent* DamagedActorFaction);

	bool IsInAir();

	FCollisionQueryParams GetQueryParams();

	UFUNCTION()
		void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

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

};

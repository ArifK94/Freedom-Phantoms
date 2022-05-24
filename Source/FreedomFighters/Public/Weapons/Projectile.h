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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		UDataTable* SurfaceImpactDatatable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Datatables", meta = (AllowPrivateAccess = "true"))
		FName RowName;
	FSurfaceImpact* SurfaceImpact;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Effects", meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* ImpactAttenuation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Effects", meta = (AllowPrivateAccess = "true"))
		USoundBase* TravelSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Damage", meta = (AllowPrivateAccess = "true"))
		bool isAnExplosive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Damage", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Damage", meta = (AllowPrivateAccess = "true"))
		float DamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Damage", meta = (AllowPrivateAccess = "true"))
		float ExplosiveRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet Damage", meta = (AllowPrivateAccess = "true"))
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet Movement", meta = (AllowPrivateAccess = "true"))
		bool UseCustomProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet Movement", meta = (AllowPrivateAccess = "true"))
		float InitialSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet Movement", meta = (AllowPrivateAccess = "true"))
		float Mass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet Movement", meta = (AllowPrivateAccess = "true"))
		float Drag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet Movement", meta = (AllowPrivateAccess = "true"))
		FVector Gravity;
	FVector Velocity;
	FVector Acceleration;
	FVector NextPosition;
	FVector PreviousPosition;

	/** Destroy projectile rather than deactivate, in case it is not used as an object pool */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Pooling", meta = (AllowPrivateAccess = "true"))
		bool DestroyOnDeactivate;

	AWeapon* WeaponParent;

	int KillCount;

	UHealthComponent* OwnerHealth;
	UTeamFactionComponent* OwnerFaction;
	ACombatCharacter* OwningCombatCharacter;

private:
	void Init();

	void RetrieveSurfaceImpactSet();

	void Movement();

	void FollowEyePoint();

	void DetectHit();

	void Explode(FVector ImpactPoint);

	FSurfaceImpactSet CheckSurface(EPhysicalSurface SurfaceType);

	void AddKill(UHealthComponent* DamagedActorHealth, UTeamFactionComponent* DamagedActorFaction);

	FCollisionQueryParams GetQueryParams();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

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

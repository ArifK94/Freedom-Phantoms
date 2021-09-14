#pragma once

#include "CoreMinimal.h"
#include "ObjectPoolActor.h"
#include "StructCollection.h"
#include "WeaponBullet.generated.h"

class UParticleSystem;
class USoundBase;
class UDamageType;
class AWeapon;
class UHealthComponent;
class UAudioComponent;
class UArrowComponent;
class USphereComponent;
class ACombatCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnKillConfirmedSignature, int, KillCount, bool, IsSingleKill, bool, IsDoubleKill, bool, IsMultiKill);
UCLASS()
class FREEDOMFIGHTERS_API AWeaponBullet : public AObjectPoolActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBullet();

	UPROPERTY(BlueprintAssignable)
		FOnKillConfirmedSignature OnKillConfirmed;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Damage", meta = (AllowPrivateAccess = "true"))
		bool IgnoreOwner;

	FTimerHandle THandler_TimeBetweenShots;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		bool ShowExplosionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		float DebugExplosionLifeTime;



	// Projectile Movement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Movement", meta = (AllowPrivateAccess = "true"))
		float InitialSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Movement", meta = (AllowPrivateAccess = "true"))
		float Mass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Movement", meta = (AllowPrivateAccess = "true"))
		float Drag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Movement", meta = (AllowPrivateAccess = "true"))
		FVector Gravity;

	FVector Velocity;

	FVector Acceleration;

	FVector NextPosition;
	FVector PreviousPosition;

	AWeapon* WeaponParent;

	int KillCount;

	ACombatCharacter* OwningCombatCharacter;
	UHealthComponent* OwnerHealth;

private:
	virtual void Activate() override;

	virtual void Deactivate() override;

	void RetrieveSurfaceImpactSet();

	void Movement();
	void DetectHit();

	void Explode(FVector ImpactPoint);

	FSurfaceImpactSet CheckSurface(EPhysicalSurface SurfaceType);

	void AddKill(UHealthComponent* DamagedActorHealth);


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

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnumCollection.h"
#include "HealthComponent.generated.h"

// OnHealthChanged Event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_NineParams(FOnHealthChangedSignature, UHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser, AWeapon*, WeaponCauser, AWeaponBullet*, Bullet, FHitResult, HitInfo);

class AWeapon;
class AWeaponBullet;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

private:
	void RegenerateHealth();

private:
	float mDeltaTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TeamFaction SelectedFaction;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool CanRegenerateHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float RegenPerSecond;

	/** Lower the damage received  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float DamageReduceFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool HasUnlimitedHealth;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isAlive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IgnoreFriendlyFire;

	/** Take damage only from explosions, can apply to vehicles  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool AcceptOnlyExplosions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		DeathType deathType;

private:
	bool HasTakenDamage;
	float RegenerationDelayAmount;
	float CurrentRegenerationDelay;

private:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Health Component")
		FOnHealthChangedSignature OnHealthChanged;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health Component")
		static bool IsFriendly(AActor* AActorA, AActor* ActorB);


	void OnDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo);

	float getCurrentHealth() {
		return Health;
	}

	bool IsAlive() {
		return isAlive;
	}

	void SetDeathType(DeathType type)
	{
		deathType = type;
	}

	void SetRegenerateHealth(bool Value) {
		CanRegenerateHealth = Value;
	}

	TeamFaction GetSelectedFaction() {
		return SelectedFaction;
	}


	void SetDamageReduceFactor(float Value) { DamageReduceFactor = Value; }
};

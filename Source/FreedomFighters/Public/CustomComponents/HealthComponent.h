// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "HealthComponent.generated.h"

// OnHealthChanged Event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, FHealthParameters, HealthParameters);

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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool IsAlive(AActor* Owner);

	void OnDamage(FHealthParameters HealthParameters);

	float GetCurrentHealth() {
		return Health;
	}

	bool IsAlive() {
		return isAlive;
	}

	void SetDeathType(DeathType type)
	{
		deathType = type;
	}

	void SetHealth(float Value) {
		Health = Value;
	}

	void SetRegenerateHealth(bool Value) {
		CanRegenerateHealth = Value;
	}

	void SetUnlimitedHealth(bool Value) {
		HasUnlimitedHealth = Value;
	}

	void SetDamageReduceFactor(float Value) { DamageReduceFactor = Value; }
};

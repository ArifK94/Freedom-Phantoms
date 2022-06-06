// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, FHealthParameters, HealthParameters);

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
	bool DefaulUnlimitedHealth;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isAlive;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool isWounded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IgnoreFriendlyFire;

	/** Take damage only from explosions, can apply to vehicles  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool AcceptOnlyExplosions;

	/** Clear all world timers for the owner when health reaches zero  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool ClearAllActorTimers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool CanBeWounded;

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

	UFUNCTION(BlueprintCallable, BlueprintPure)
		static bool IsWounded(AActor* Owner);

	static void SetIsReviving(AActor* Owner, bool Value);


	void OnDamage(FHealthParameters HealthParameters);

	void Revive();

	float GetCurrentHealth() {
		return Health;
	}

	bool IsAlive() {
		return isAlive;
	}

	void SetIsAlive(bool Value) {
		isAlive = Value;
	}

	bool GetIsWounded() {
		return isWounded;
	}

	bool GetDefaulUnlimitedHealth() {
		return DefaulUnlimitedHealth;
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

	void SetCanBeWounded(bool Value) { CanBeWounded = Value; }

};

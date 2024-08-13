// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, FHealthParameters, HealthParameters);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

private:
	void RegenerateHealth();

private:
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
	bool CanBeWoundedDefault;

	/** Which classes do not affect this health component?  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<AActor>> ImmuneActorClasses;

private:
	float RegenerationDelayAmount;

	UPROPERTY()
		FTimerHandle THandler_Regeneration;

private:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Health Component")
		FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		static bool IsActorAlive(AActor* Owner);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		static bool IsActorWounded(AActor* Owner);

	static void SetIsReviving(AActor* Owner, bool Value);


	UFUNCTION(BlueprintCallable)
		void OnDamage(FHealthParameters HealthParameters);

	UFUNCTION(BlueprintCallable)
		static void ApplyDamage(FHealthParameters HealthParameters);

	UFUNCTION(BlueprintCallable)
		static void ApplyExplosionDamage(AActor* DamageCauser, FVector ImpactPoint, float Radius, float Damage);

	void SetCanBeWounded(bool Value);


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


	bool GetCanBeWounded() { return CanBeWounded; }


	bool GetCanBeWoundedDefault() { return CanBeWoundedDefault; }


};

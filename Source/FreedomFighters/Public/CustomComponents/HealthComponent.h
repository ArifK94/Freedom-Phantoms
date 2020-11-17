// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// OnHealthChanged Event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);


UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class TeamFaction : uint8
{
	Neutral			UMETA(DisplayName = "Neutral"),
	ShadowCompany	UMETA(DisplayName = "ShadowCompany"),
	Russian 		UMETA(DisplayName = "Russian"),
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class DeathType : uint8
{
	FleshDefault	UMETA(DisplayName = "FleshDefault"),
	FleshVulnerable	UMETA(DisplayName = "FleshVulnerable"),
	Head			UMETA(DisplayName = "Head"),
	Groin 			UMETA(DisplayName = "Groin"),
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health Component")
		TeamFaction SelectedFaction;

private:
	void RegenerateHealth();


private:
	float mDeltaTime;

	UPROPERTY(BlueprintReadOnly, Category = "Health Component", meta = (AllowPrivateAccess = "true"))
		float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Component", meta = (AllowPrivateAccess = "true"))
		float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Component", meta = (AllowPrivateAccess = "true"))
		bool CanRegenerateHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Component", meta = (AllowPrivateAccess = "true"))
		float RegenPerSecond;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Component", meta = (AllowPrivateAccess = "true"))
		bool HasUnlimitedHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Health Component", meta = (AllowPrivateAccess = "true"))
		bool isAlive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		DeathType deathType;

private:
	FHitResult HitInfo;


private:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Health Component")
		void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:


	UPROPERTY(BlueprintAssignable, Category = "Health Component")
		FOnHealthChangedSignature OnHealthChanged;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health Component")
		static bool IsFriendly(AActor* AActorA, AActor* ActorB);


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

	void SetHitInfo(FHitResult Hit)
	{
		HitInfo = Hit;
	}
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterComponent.generated.h"


class AWeapon;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UShooterComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	AWeapon* m_Weapon;

	FTimerHandle THandler_BeginShoot;
	FTimerHandle THandler_ResumeShoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float m_TimeBetweenShotsMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float m_TimeBetweenShotsMax;


public:	
	UShooterComponent();

	UFUNCTION(BlueprintCallable)
		void BeginFire();

	UFUNCTION(BlueprintCallable)
		void PauseFire();

	UFUNCTION(BlueprintCallable)
		void EndFire();

	UFUNCTION(BlueprintCallable)
		void SetWeapon(AWeapon* InWeapon);

protected:
	virtual void BeginPlay() override;

public:
	float TimeBetweenShotsMin() { return m_TimeBetweenShotsMin; }
	float TimeBetweenShotsMax() { return m_TimeBetweenShotsMax; }

	void SetTimeBetweenShotsMin(float Value) { m_TimeBetweenShotsMin = Value; }
	void SetTimeBetweenShotsMax(float Value) { m_TimeBetweenShotsMax = Value; }

};

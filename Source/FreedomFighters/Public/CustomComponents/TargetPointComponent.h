// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetPointComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UTargetPointComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	/** Should be used if all target points are occupied   */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsFallbackPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsPointTaken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName PointName;

	class UHealthComponent* HealthComp;

public:	
	UTargetPointComponent();

	UFUNCTION(BlueprintCallable)
		void SetPoint(AActor* Owner);

private:
	UFUNCTION(BlueprintCallable)
		void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo);

protected:
	virtual void BeginPlay() override;
		
};

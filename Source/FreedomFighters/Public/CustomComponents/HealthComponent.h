// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// OnHealthChanged Event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health Component")
		uint8 TeamNumber;

protected:
	virtual void BeginPlay() override;


	UPROPERTY(BlueprintReadOnly, Category = "Health Component")
		float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Component")
		float MaxHealth;


	UFUNCTION(BlueprintCallable, Category = "Health Component")
		void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:


	UPROPERTY(BlueprintAssignable, Category = "Health Component")
		FOnHealthChangedSignature OnHealthChanged;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health Component")
		static bool IsFriendly(AActor* AActorA, AActor* ActorB);
};

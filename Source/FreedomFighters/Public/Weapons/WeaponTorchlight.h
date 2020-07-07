// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTorchlight.generated.h"

UCLASS()
class FREEDOMFIGHTERS_API AWeaponTorchlight : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Accessory", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* MeshComp;

public:	
	AWeaponTorchlight();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};

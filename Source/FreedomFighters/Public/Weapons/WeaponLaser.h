// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponLaser.generated.h"

UCLASS()
class FREEDOMFIGHTERS_API AWeaponLaser : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Accessory", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* MeshComp;

public:	
	AWeaponLaser();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};

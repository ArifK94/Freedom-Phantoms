// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "AssaultRifle.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMPHANTOMS_API AAssaultRifle : public AWeapon
{
	GENERATED_BODY()

public:
	AAssaultRifle();
	
private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Components", meta = (AllowPrivateAccess = "true"))
		TArray<UStaticMesh*> BarrelMeshes;

	UStaticMeshComponent* BarrelComp;

private:

	virtual void BeginPlay() override;

	virtual FVector getMuzzleLocation() override;

	void setBarrel();
};

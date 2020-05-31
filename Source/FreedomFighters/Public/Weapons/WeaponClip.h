// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponClip.generated.h"

UCLASS()
class FREEDOMFIGHTERS_API AWeaponClip : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponClip();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* clipMeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		int ammoCapacity;

protected:
	virtual void BeginPlay() override;


public:
	virtual void Tick(float DeltaTime) override;

	UStaticMeshComponent* getClipMesh() { return clipMeshComp; }
	int GetAmmoCapacity() const { return ammoCapacity; }


};

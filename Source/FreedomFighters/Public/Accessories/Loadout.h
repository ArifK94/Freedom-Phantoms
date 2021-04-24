// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Accessories/Accessory.h"
#include "EnumCollection.h"
#include "Loadout.generated.h"

class USkeletalMeshComponent;
class UWeaponSet;
UCLASS()
class FREEDOMFIGHTERS_API ALoadout : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		LoadoutType loadoutType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		TArray<FName> PhysicsBones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		bool UseMasterPoseComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh;


private:
	 UWeaponSet* CurrentWeaponSetObj;


public:	
	ALoadout();
	void Init(UWeaponSet* WeaponSetObj);

	class AWeapon* SpawnPrimaryWeapon(USkeletalMeshComponent* mesh, AActor* owner);

	USkeletalMeshComponent* GetMesh() { return Mesh; }

private:
	virtual void BeginPlay() override;

	void SimlateBones();

public:
	bool IsUseMasterPoseComponent() {
		return UseMasterPoseComponent;
	}
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Accessories/Accessory.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "Loadout.generated.h"

class USkeletalMeshComponent;
class AWeapon;
class AThrowableWeapon;
UCLASS()
class FREEDOMPHANTOMS_API ALoadout : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		LoadoutType loadoutType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		TArray<FName> PhysicsBones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		bool UseMasterPoseComponent;

	/** Use Parent Mesh Component instead of using this actor's Mesh for all the logic. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		bool UseParentMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh;

public:	
	ALoadout();

	/** If primary is true, then spawn primary weapon otherwise spawn secondary */
	AWeapon* SpawnWeapon(FWeaponsSet* WeaponsDataSet, bool IsPrimary);

	/** If primary is true, then spawn primary weapon otherwise spawn secondary */
	AWeapon* SpawnWeapon(TSubclassOf<AWeapon> WeaponClass, bool IsPrimary);

	AThrowableWeapon* SpawnGrenade(FWeaponsSet* WeaponsDataSet);

	UFUNCTION(BlueprintCallable)
		void HolsterWeapon(AWeapon* Weapon);

	USkeletalMeshComponent* GetMesh();

private:
	virtual void BeginPlay() override;

	void SimlateBones();

public:
	bool IsUseMasterPoseComponent() {
		return UseMasterPoseComponent;
	}
};

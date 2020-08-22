// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Accessories/Accessory.h"
#include "Loadout.generated.h"

class USkeletalMeshComponent;
class UWeaponSet;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class LoadoutType : uint8
{
	Assault UMETA(DisplayName = "Assault"),
	SMG 	UMETA(DisplayName = "SMG"),
	Shotgun UMETA(DisplayName = "Shotgun"),
	LMG		UMETA(DisplayName = "LMG")
};

UCLASS()
class FREEDOMFIGHTERS_API ALoadout : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		LoadoutType loadoutType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh;


private:
	//class UGameInstanceController* gameInstanceController;

	 UWeaponSet* CurrentWeaponSetObj;


public:	
	ALoadout();
	void Init(UWeaponSet* WeaponSetObj);

	class AWeapon* SpawnPrimaryWeapon(USkeletalMeshComponent* mesh, AActor* owner);

	USkeletalMeshComponent* getLoadoutMesh() { return Mesh; }

protected:

	virtual void BeginPlay() override;


};

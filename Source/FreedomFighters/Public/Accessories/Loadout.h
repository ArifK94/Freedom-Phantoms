// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Accessories/Accessory.h"
#include "Loadout.generated.h"

class USkeletalMeshComponent;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class LoadoutType : uint8
{
	Assault UMETA(DisplayName = "Assault"),
	SMG 	UMETA(DisplayName = "SMG"),
	Shotgun UMETA(DisplayName = "Shotgun"),
	LMG		UMETA(DisplayName = "LMG")
};

UCLASS()
class FREEDOMFIGHTERS_API ALoadout : public AAccessory
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		FVector PrimaryWeaponHolsterLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		FRotator PrimaryWeaponHolsterRotation;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		FVector SideArmHolsterLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		FRotator SideArmHolsterRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout", meta = (AllowPrivateAccess = "true"))
		LoadoutType loadoutType;
	

private:
	class UGameInstanceController* gameInstanceController;


public:	
	ALoadout();

	class AWeapon* SpawnPrimaryWeapon(USkeletalMeshComponent* mesh, AActor* owner);

	FVector getPrimaryWeaponHolsterLocation();
	FRotator getPrimaryWeaponHolsterRotation();

	FVector getSideArmHolsterLocation();
	FRotator getSideArmHolsterRotation();

	USkeletalMeshComponent* getLoadoutMesh() { return SkelMesh; }

protected:

	virtual void BeginPlay() override;


};

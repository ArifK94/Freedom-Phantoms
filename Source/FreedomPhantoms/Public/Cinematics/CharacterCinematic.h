// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterCinematic.generated.h"

class USkeletalMeshComponent;
class AWeapon;
class UChildActorComponent;
UCLASS()
class FREEDOMPHANTOMS_API ACharacterCinematic : public AActor
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* BodyMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* HeadMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* LoaoutMesh;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* PrimaryWeaponActorComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* HolsterWeaponActorComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	FName WeaponHandSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	bool ShowPrimaryWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	bool ShowSecondaryWeapon;

	/** Use the follow camera for the eye view point where the weapon can shoot towards. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	bool UseFollowCameraViewPoiint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
	FName HeadSocket;

	/**
	* Fix body transform to avoid manually constantly fix mesh transform in the world. 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	bool FixTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	FName SidearmHolsterSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	FName BackHolsterSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	FName Holster1Socket;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	bool UseSidearmHolster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	bool UseBackHolster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	bool UseHolster1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	bool UsePrimarySidearmHolster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	bool UsePrimaryBackHolster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	bool UsePrimaryHolster;
	
public:	
	ACharacterCinematic();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponHand(AActor* ParentWeapon, FName NewHandSocket);

	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const;

protected:
	virtual void BeginPlay() override;

private:
	void OnConstruction(const FTransform& Transform) override;


#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


	void Update();

	void AttachWeaponHolster(AActor* ParentWeapon, bool CanUseSidearmHolster, bool CanUseBackHolster, bool CanUseHolster1, bool HoldWeapon);

	void FixBodyTransform();
};

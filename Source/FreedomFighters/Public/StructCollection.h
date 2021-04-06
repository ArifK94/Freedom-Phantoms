// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumCollection.h"
#include "StructCollection.generated.h"

class ABaseCharacter;
class AAircraft;
class ATargetSystemMarker;
class AWeapon;
class UUserWidget;

USTRUCT(BlueprintType)
struct FAircraftSeating
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		EAircraftRole Role;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName SeatingSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 SeatPosition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 RappelAnimIndex;

	/** Check if rope is occupied */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool isRopeLeftSide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float CameraViewYawMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float CameraViewYawMax;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ABaseCharacter> Character;
	ABaseCharacter* CharacterObj;


	AAircraft* OwningAircraft;

	FAircraftSeating()
	{

	}
};

USTRUCT(BlueprintType)
struct FAircraftWeapon
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<AWeapon> Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName WeaponSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName CameraSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UUserWidget> HUD;

	FAircraftWeapon()
	{

	}
};

USTRUCT(BlueprintType)
struct FTargetSystemNode
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		ABaseCharacter* Character;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		ATargetSystemMarker* Marker; // the marker class containing the widget component

	FTargetSystemNode()
	{

	}
};
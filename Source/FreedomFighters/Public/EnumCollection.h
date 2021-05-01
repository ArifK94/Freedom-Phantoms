// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


UENUM(BlueprintType)
enum class WeaponType : uint8
{
	Rifle		UMETA(DisplayName = "Rifle"),
	SMG 		UMETA(DisplayName = "SMG"),
	Shotgun		UMETA(DisplayName = "Shotgun"),
	LMG			UMETA(DisplayName = "LMG"),
	Pistol		UMETA(DisplayName = "Pistol")
};

UENUM(BlueprintType)
enum class SelectiveFire : uint8
{
	Automatic		UMETA(DisplayName = "Automatic"),
	SemiAutomatic 	UMETA(DisplayName = "SemiAutomatic"),
	Burst			UMETA(DisplayName = "Burst")
};


UENUM(BlueprintType)
enum class CoverType : uint8
{
	Default			UMETA(DisplayName = "Default"),
	CornerLeft		UMETA(DisplayName = "CornerLeft"),
	CornerRight 	UMETA(DisplayName = "CornerRight")
};

UENUM(BlueprintType)
enum class LoadoutType : uint8
{
	Assault UMETA(DisplayName = "Assault"),
	SMG 	UMETA(DisplayName = "SMG"),
	Shotgun UMETA(DisplayName = "Shotgun"),
	LMG		UMETA(DisplayName = "LMG")
};

UENUM(BlueprintType)
enum class EAircraftMovement : uint8
{
	Grounded		UMETA(DisplayName = "Grounded"),
	Hovering		UMETA(DisplayName = "Hovering"),
	Rappel			UMETA(DisplayName = "Rappel"),
	Stopping 		UMETA(DisplayName = "Stopping"),
	MovingForward	UMETA(DisplayName = "MovingForward")
};


UENUM(BlueprintType)
enum class EAircraftRole : uint8
{
	Pilot				UMETA(DisplayName = "Pilot"),
	SideGunner 			UMETA(DisplayName = "SideGunner"),
	MountedGunnner		UMETA(DisplayName = "MountedGunnner")
};
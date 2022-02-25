// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class TeamFaction : uint8
{
	Neutral			UMETA(DisplayName = "Neutral"),
	ShadowCompany	UMETA(DisplayName = "ShadowCompany"),
	Russian 		UMETA(DisplayName = "Russian")
};

UENUM(BlueprintType)
enum class DeathType : uint8
{
	FleshDefault	UMETA(DisplayName = "FleshDefault"),
	FleshVulnerable	UMETA(DisplayName = "FleshVulnerable"),
	Head			UMETA(DisplayName = "Head"),
	Groin 			UMETA(DisplayName = "Groin")
};

UENUM(BlueprintType)
enum class WeaponType : uint8
{
	Rifle		UMETA(DisplayName = "Rifle"),
	SMG 		UMETA(DisplayName = "SMG"),
	Shotgun		UMETA(DisplayName = "Shotgun"),
	LMG			UMETA(DisplayName = "LMG"),
	Pistol		UMETA(DisplayName = "Pistol"),
	MountedGun	UMETA(DisplayName = "MountedGun")
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
enum class EVehicleMovement : uint8
{
	Grounded		UMETA(DisplayName = "Grounded"),
	Hovering		UMETA(DisplayName = "Hovering"),
	Rappel			UMETA(DisplayName = "Rappel"),
	Stopping 		UMETA(DisplayName = "Stopping"),
	MovingForward	UMETA(DisplayName = "MovingForward"),
	Waiting			UMETA(DisplayName = "Waiting")
};

UENUM(BlueprintType)
enum class EVehicleSpeedType : uint8
{
	Normal		UMETA(DisplayName = "Normal"),
	Specified	UMETA(DisplayName = "Specified")
};


UENUM(BlueprintType)
enum class EVehicleRole : uint8
{
	Pilot				UMETA(DisplayName = "Pilot"),
	SideGunner 			UMETA(DisplayName = "SideGunner"),
	MountedGunnner		UMETA(DisplayName = "MountedGunnner")
};

UENUM(BlueprintType)
enum class CommanderOrders : uint8
{
	Follow		UMETA(DisplayName = "Follow"),
	Attack		UMETA(DisplayName = "Attack"),
	Defend 		UMETA(DisplayName = "Defend")
};

UENUM(BlueprintType)
enum class AIBehaviourState : uint8
{
	Normal					UMETA(DisplayName = "Normal"),
	Patrol					UMETA(DisplayName = "Patrol"),
	MovingToLastSeenEnemy	UMETA(DisplayName = "MovingToLastSeenEnemy"),
	PriorityDestination		UMETA(DisplayName = "PriorityDestination"),
	PriorityOrdersCommander	UMETA(DisplayName = "PriorityOrdersCommander"),
};
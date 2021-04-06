// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class CoverType : uint8
{
	Default			UMETA(DisplayName = "Default"),
	CornerLeft		UMETA(DisplayName = "CornerLeft"),
	CornerRight 	UMETA(DisplayName = "CornerRight")
};


UENUM(BlueprintType)
enum class EAircraftMovement : uint8
{
	Grounded		UMETA(DisplayName = "Grounded"),
	Hovering		UMETA(DisplayName = "Hovering"),
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
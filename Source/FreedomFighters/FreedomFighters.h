// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define SURFACE_FLESHDEFAULT		SurfaceType1
#define SURFACE_FLESHVULNERABLE		SurfaceType2
#define SURFACE_HEAD				SurfaceType3
#define SURFACE_GROIN				SurfaceType4

#define COLLISION_WEAPON			ECC_GameTraceChannel1


//UENUM(BlueprintType)
//enum class EAircraftMovement : uint8
//{
//	Grounded		UMETA(DisplayName = "Grounded"),
//	Hovering		UMETA(DisplayName = "Hovering"),
//	Stopping 		UMETA(DisplayName = "Stopping"),
//	MovingForward	UMETA(DisplayName = "MovingForward")
//};
//
//
//UENUM(BlueprintType)
//enum class EAircraftRole : uint8
//{
//	Pilot				UMETA(DisplayName = "Pilot"),
//	SideGunner 			UMETA(DisplayName = "SideGunner"),
//	MountedGunnner		UMETA(DisplayName = "MountedGunnner")
//};

//////////////////////// ----------------------------------- STRUCTS ----------------------------------- ////////////////////////

//#pragma region Structs
//
//USTRUCT(BlueprintType)
//struct FAircraftSeating
//{
//	GENERATED_BODY()
//
//public:
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//		EAircraftRole Role;
//
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//		FName SeatingSocketName;
//
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//		int32 SeatPosition;
//
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//		int32 RappelAnimIndex;
//
//	/** Check if rope is occupied */
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//		bool isRopeLeftSide;
//
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//		float CameraViewYawMin;
//
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//		float CameraViewYawMax;
//
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//		TSubclassOf<ABaseCharacter> Character;
//	ABaseCharacter* CharacterObj;
//
//
//	AAircraft* OwningAircraft;
//
//	FAircraftSeating()
//	{
//
//	}
//};
//
//#pragma endregion
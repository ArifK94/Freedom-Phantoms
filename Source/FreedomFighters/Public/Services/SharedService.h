// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

/**
 * Any shared functions which can be used across any class.
 */
class FREEDOMFIGHTERS_API SharedService
{
public:

	/**
	* To help AI throw projectiles at an angle to the target. This is done by adjusting the pitch accordingly.
	* Returns a bool to check if angle to target is reachable
	*/
	static bool ThrowRotationAngle(FVector Start, FVector End, FRotator& TargetRotation);


	/**
	* Check if TargetActor is behind ActorA
	*/
	static bool IsTargetBehind(AActor* ActorA, AActor* TargetActor);

	/**
	* Is an actor near a location?
	* Radius is how many far apart should be considered nearby to the location.
	*/
	static bool IsNearTargetPosition(FVector Start, FVector Location, float Radius);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"

#include "SharedService.generated.h"

/**
 * Any shared functions which can be used across any class.
 */
UCLASS(Blueprintable)
class FREEDOMPHANTOMS_API USharedService : public UObject
{
	GENERATED_BODY()
	
public:
	USharedService();

	/**
	* To help AI throw projectiles at an angle to the target. This is done by adjusting the pitch accordingly.
	* Returns a bool to check if angle to target is reachable
	*/
	UFUNCTION(BlueprintCallable)
		static bool ThrowRotationAngle(FVector Start, FVector End, FRotator& TargetRotation);


	/**
	* Check if TargetActor is behind ActorA
	*/
	UFUNCTION(BlueprintCallable)
		static bool IsTargetBehind(AActor* ActorA, AActor* TargetActor, float Amount = -.7f);

	/**
	* Is an actor near a location?
	* Radius is how many far apart should be considered nearby to the location.
	*/
	UFUNCTION(BlueprintCallable)
		static bool IsNearTargetPosition(FVector Start, FVector Location, float Radius);

	/**
	* are actors near each other?
	*/
	static bool IsNearTargetPosition(AActor* ActorA, AActor* ActorB, float Radius);


	/**
	* Can the target actor been seen from a position?
	*/
	UFUNCTION(BlueprintCallable)
		static bool CanSeeTarget(UWorld* World, FVector Start, AActor* TargetActor, AActor* Owner);

	/**
	* Check if an actor is in the air.
	*/
	UFUNCTION(BlueprintCallable)
		static bool IsInAir(struct FHitResult& OutHit, AActor* Actor, float Length = 100.f);

	/**
	* Destroy an actor component if it exists.
	*/
	UFUNCTION(BlueprintCallable)
		static void DestroyActorComponent(UActorComponent* ActorComponent);


	/**
	* Can an actor be seen on the screen?
	* Add offset of actor's screen location.
	*/
	UFUNCTION(BlueprintCallable)
		static bool IsActorOnScreen(UObject* WorldContextObject, AActor* Actor, FVector2D Offset = FVector2D::ZeroVector);

};

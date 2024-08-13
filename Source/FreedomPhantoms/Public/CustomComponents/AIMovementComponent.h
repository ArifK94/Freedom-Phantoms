// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumCollection.h"
#include "CustomComponents/Engine/MyActorComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestinationSetSignature, AIBehaviourState, BehaviourState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestinationReachedSignature, FVector, TargetDestination);

class USphereComponent;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UAIMovementComponent : public UMyActorComponent
{
	GENERATED_BODY()

private:

	UPROPERTY()
		class AAIController* AIController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TEnumAsByte<EPathFollowingRequestResult::Type> CurrentMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float MinAcceptanceRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool StopOnOverlap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool UsePathfinding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool ProjectDestinationToNavigation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool CanStrafe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool AllowPartialPaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class UNavigationQueryFilter> FilterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		float MovementDebugLifeTime;


public:	
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnDestinationSetSignature OnDestinationSet;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnDestinationReachedSignature OnDestinationReached;

	UAIMovementComponent();

protected:
	virtual void Init() override;

public:	
	UFUNCTION(BlueprintCallable)
		EPathFollowingRequestResult::Type MoveToDestination(FVector TargetDestination, float AcceptRadius, AIBehaviourState BehaviourState, bool SprintToTarget = true, bool WalkNearTarget = true);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector FindNearbyDestinationPoint(FVector TargetDestination, float Radius, TArray<AActor*> IgnoreActors);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector ValidateDestination(FVector Location, bool& IsLocationValid);

	EPathFollowingRequestResult::Type GetCurrentMovement() { return CurrentMovement; }
		
};

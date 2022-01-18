// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumCollection.h"
#include "Components/ActorComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestinationSetSignature, AIBehaviourState, BehaviourState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestinationReachedSignature, FVector, TargetDestination);

class USphereComponent;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UAIMovementComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	class AAIController* AIController;
	class ABaseCharacter* Character;
	APawn* PawnOwner;

	bool IsDestinationSet;

	USphereComponent* DestinationTrigger;

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
		float MovementDebugLifetTime;

public:	
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnDestinationSetSignature OnDestinationSet;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnDestinationReachedSignature OnDestinationReached;

	UAIMovementComponent();

	void Init();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	UFUNCTION(BlueprintCallable)
		EPathFollowingRequestResult::Type MoveToDestination(FVector TargetDestination, float AcceptRadius, bool SprintToTarget = true, bool WalkNearTarget = true);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector ValidateDestination(FVector Location, bool& IsLocationValid);
		
};

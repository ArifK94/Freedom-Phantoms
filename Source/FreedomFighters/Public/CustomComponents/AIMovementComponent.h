// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIMovementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UAIMovementComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	class AAIController* AIController;
	class ABaseCharacter* Character;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float AcceptanceRadius;

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

public:	
	UAIMovementComponent();

protected:
	virtual void BeginPlay() override;

public:	
	UFUNCTION(BlueprintCallable)
		EPathFollowingRequestResult::Type MoveToDestination(FVector TargetDestination, float AcceptRadius, bool WalkNearTarget = true);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector ValidateDestination(FVector Location, bool& IsLocationValid);
		
};

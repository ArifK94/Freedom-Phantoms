// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StructCollection.h"
#include "TargetFinderComponent.generated.h"

class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetSearchSignature, FTargetSearchParameters, TargetSearchParameters);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UTargetFinderComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	USphereComponent* TargetSightSphere;

	FTimerHandle THandler_TargetSearch;

	/**
	* Hold the last enemy seen.
	*/
	AActor* LastSeenEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool FindTargetPerFrame;

	/** Enable to search for all overlapped targets in sphere radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool CreateTargetSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TargetSightRadius;

	/** Limit the number of targets the component can process */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int FinderLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool ShowDebugTrace;

	/** Type of actors to accept. Empty list will return all actor classes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<AActor>> ClassFilters;

	/** Type of actors to ignore for the line trace for objects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<AActor>> IgnoreActorClasses;

	/** Type of actors to ignore for the line trace for objects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<AActor*> IgnoreActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<TEnumAsByte<EObjectTypeQuery>> CollisionChannels;

public:	
	UTargetFinderComponent();

	void Init();

	UFUNCTION(BlueprintCallable)
		AActor* FindTarget();

	bool GetTrace(FHitResult& OutHit, FVector Start, FVector End);

	FRotator RotateTowardsTarget(AActor* OwnerActor, AActor* TargetActor, FRotator CurrentRotation, FRotator& TargetRotation, float DeltaTime, float LerpSpeed);

	bool IsTargetBehind(AActor* ActorA, AActor* TargetActor);

	bool DoesClassFilterExist(TSubclassOf<AActor> Class);

private:
	void FindTargetUpdate();

	bool CanSeeLastTarget();

	/** Some actors such as vehicles may have enemy characters attached as children, so would need check if any enemy children exist if parent actor is not an enemy */
	AActor* GetChildrenTargets(AActor* ParentTarget);

	bool IsActorFiltered(AActor* Actor);

	bool IsActorToIgnore(AActor* Actor);

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnTargetSearchSignature OnTargetSearch;

	void SetFindTargetPerFrame(bool Value);

	void SetCreateTargetSphere(bool Value) { CreateTargetSphere = Value; }

	void AddClassFilter(TSubclassOf<AActor> Class) { ClassFilters.Add(Class); }
	
	void AddIgnoreClass(TSubclassOf<AActor> Class) { IgnoreActorClasses.Add(Class); }

	void AddIgnoreActor(AActor* Actor) { IgnoreActors.Add(Actor); }


	bool CanSeeTarget(AActor* TargetActor, FVector& TargetLocation);

	UFUNCTION(BlueprintCallable)
		static void AddIgnoreClass(AActor* InOwner, TSubclassOf<AActor> InClass);

	UFUNCTION(BlueprintCallable)
		static void AddIgnoreActor(AActor* InOwner, AActor* InActorIgnore);

		
};

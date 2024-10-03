// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomComponents/Engine/MyActorComponent.h"
#include "StructCollection.h"
#include "TargetFinderComponent.generated.h"

class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetSearchSignature, FTargetSearchParameters, TargetSearchParameters);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UTargetFinderComponent : public UMyActorComponent
{
	GENERATED_BODY()

private:

	UPROPERTY()
		USphereComponent* TargetSightSphere;

	UPROPERTY()
		FTimerHandle THandler_CountdownTargetLost;

	/**
	* Hold the last enemy seen.
	*/
	UPROPERTY()
		AActor* LastSeenTarget;

	/**
	* Actors which have been set as non-targets e.g. friendlies should not be processed again to avoid performance issues.
	*/
	UPROPERTY()
		TArray<FString> ProcessedIgnoreActors;

	UPROPERTY()
		FTargetSearchParameters LastSeenTargetParam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TargetSightRadius;

	/** Maximum sight distance to see target that has been already seen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float LoseSightRadius;

	/** Limit the number of targets the component can process */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int FinderLimit;

	/** Cooldown timer for target to disappear if no other target found. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CountdownTargetLost;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool FindTargetPerFrame;

public:	
	UTargetFinderComponent();

	UFUNCTION(BlueprintCallable)
		AActor* FindTarget();

	bool GetTrace(FHitResult& OutHit, FVector Start, FVector End, AActor* TargetActor);

	FRotator RotateTowardsTarget(AActor* OwnerActor, AActor* TargetActor, FRotator CurrentRotation, FRotator& TargetRotation, float DeltaTime, float LerpSpeed);

	bool IsTargetBehind(AActor* ActorA, AActor* TargetActor);

	bool DoesClassFilterExist(TSubclassOf<AActor> Class);

	void RemoveClassFilter(TSubclassOf<AActor> Class);

private:
	virtual void Init() override;

	TArray<AActor*> GetActorsInRadius(float Radius);

	bool CanSeeLastTarget();

	/** Some actors such as vehicles may have enemy characters attached as children, so would need check if any enemy children exist if parent actor is not an enemy */
	AActor* GetChildrenTargets(AActor* ParentTarget);

	bool IsActorFiltered(AActor* Actor);

	bool IsActorToIgnore(AActor* Actor);

	/** Remove last seen target */
	void ClearLastSeenTarget();

	void AddToIgnoreProcessed(AActor* Actor);

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnTargetSearchSignature OnTargetSearch;

	/** Do not call this method in the constructor since the timer will be set and it will cause the application to crash. */
	UFUNCTION(BlueprintCallable)
		void SetFindTargetPerFrame(bool Value);

	void AddClassFilter(TSubclassOf<AActor> Class);
	
	void AddIgnoreClass(TSubclassOf<AActor> Class);

	void AddIgnoreActor(AActor* Actor);

	void RemoveIgnoreActor(AActor* Actor);

	bool CanSeeTarget(AActor* TargetActor, FVector& TargetLocation);

	UFUNCTION(BlueprintCallable)
		static void AddIgnoreClass(AActor* InOwner, TSubclassOf<AActor> InClass);

	UFUNCTION(BlueprintCallable)
		static void AddIgnoreActor(AActor* InOwner, AActor* InActorIgnore);

		
};

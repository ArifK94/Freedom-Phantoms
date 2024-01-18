// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomComponents/Engine/MyActorComponent.h"
#include "OptimizerComponent.generated.h"

/**
* Optimize the owning actor & its components for performance.
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UOptimizerComponent : public UMyActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
		float DefaultActorTickInterval;

	/** Add a delay at the beginning so the actor can completely render its properties. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float BeginPlayDelayAmount;
		FTimerHandle THandler_BeginPlay;


	/** Can the owning actor have its tick optimized? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick", meta = (AllowPrivateAccess = "true"))
		bool CanOptimizeTick;

	/** Add more optimization to the actor by affecting its children components. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick", meta = (AllowPrivateAccess = "true"))
		bool CanOptimizeChildrenComponents;

	/** Add offset of actor's screen location.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick", meta = (AllowPrivateAccess = "true"))
		FVector2D OptimizeOffset;

	/** The radius size which the owner can have its tick back to normal when within player radius. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick", meta = (AllowPrivateAccess = "true"))
		float TickRadius;

	/** The interval of the actor's tick when optimized. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick", meta = (AllowPrivateAccess = "true"))
		float TickIntervalOptimized;

	/** Should the actor continue to be optimized from a distance if currently on screen? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick Distance", meta = (AllowPrivateAccess = "true"))
		bool ApplyDistanceOptimization;

	/** The minimum distance for the actor to be optimized. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick Distance", meta = (AllowPrivateAccess = "true"))
		float MinimumDistanceOptimization;

public:
	/** Some components may not need to be optimized and should run as normal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick", meta = (AllowPrivateAccess = "true"))
		TSet<TSubclassOf<UActorComponent>> IgnoredComponentClasses;

public:	
	UOptimizerComponent();

	/**
	* Get the tick interval of an actor when on / off screen.
	* Increase interval when off-screen.
	* Default back to interval when back on-screen.
	*/
	void GetOptimizedTick(float& NewTickInterval);


	/**
	* Optimize the actor's tick.
	*/
	void OptimizeActorTick(float NewTickInterval);

	/**
	* Optimize the tick of the actor's components.
	*/
	void OptimizeComponentsTick(float NewTickInterval);

protected:		
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	void BeginPlayDelay();
};

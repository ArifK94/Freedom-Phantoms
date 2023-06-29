// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "StructCollection.h"
#include "TargetPointComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UTargetPointComponent : public USceneComponent
{
	GENERATED_BODY()

private:
	/** Should be used if all target points are occupied   */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsFallbackPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsPointTaken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName PointName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AActor* CurrentOwner;

	UPROPERTY()
		class USphereComponent* SphereComponent;

	UPROPERTY()
		class UArrowComponent* ArrowComponent;

	UPROPERTY()
		class UHealthComponent* HealthComp;

public:	
	UTargetPointComponent();

	UFUNCTION(BlueprintCallable)
		void SetPoint(AActor* Owner);

private:
	UFUNCTION(BlueprintCallable)
		void OnHealthChanged(FHealthParameters InHealthParameters);
		
};

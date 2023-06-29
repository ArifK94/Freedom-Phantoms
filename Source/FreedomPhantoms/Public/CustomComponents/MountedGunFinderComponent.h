// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MountedGunFinderComponent.generated.h"


class AMountedGun;
class USphereComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UMountedGunFinderComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	UPROPERTY()
		USphereComponent* SearchSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float SearchRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int SearchLimit;

	float DefaultSearchRadius;

	UPROPERTY()
		APawn* OwningPawn;

public:	
	UMountedGunFinderComponent();

	void Init();

	AMountedGun* FindMG();

	void FocusTarget(AMountedGun* MG, FVector Location);

	UFUNCTION(BlueprintCallable)
		static void FocusTarget(AActor* Owner, AMountedGun* MG, FVector Location);

	bool IsInTargetRange(AMountedGun* MG, FVector StartLocation, FVector TargetLocation);

	bool IsInTargetRange(AMountedGun* MG, AActor* StartActor, AActor* TargetActor);

protected:
	virtual void BeginPlay() override;

public:
	void ResetSearchRadius();

	void SetSearchRadius(float Radius) { SearchRadius = Radius; }
		
};

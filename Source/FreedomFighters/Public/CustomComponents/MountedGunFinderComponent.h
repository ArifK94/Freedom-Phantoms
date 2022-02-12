// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MountedGunFinderComponent.generated.h"


class AMountedGun;
class USphereComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UMountedGunFinderComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	USphereComponent* SearchSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float SearchRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int SearchLimit;

public:	
	UMountedGunFinderComponent();

	void Init();

	AMountedGun* FindMG();

	void FocusTarget(AMountedGun* MG, FVector Location);

	bool IsInTargetRange(AMountedGun* MG, FVector StartLocation, FVector TargetLocation);

protected:
	virtual void BeginPlay() override;

		
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MountedGunFinderComponent.generated.h"


class AMountedGun;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UMountedGunFinderComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float SearchRadius;

public:	
	UMountedGunFinderComponent();

	AMountedGun* FindMG();

protected:
	virtual void BeginPlay() override;

		
};

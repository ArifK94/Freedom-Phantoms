// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "HelicopterAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMPHANTOMS_API UHelicopterAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

		virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Notify", meta = (AllowPrivateAccess = "true"))
		bool DetachFromHelicopter;


	/** To allow another character rappel down */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Notify", meta = (AllowPrivateAccess = "true"))
		bool IsRopeFreeToUse;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CharacterAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMPHANTOMS_API UCharacterAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
		virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Notify", meta = (AllowPrivateAccess = "true"))
		bool MoveBackToCover;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Notify", meta = (AllowPrivateAccess = "true"))
		bool ShouldCrouch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Notify", meta = (AllowPrivateAccess = "true"))
		bool AlignHandguardIK;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Notify", meta = (AllowPrivateAccess = "true"))
		bool StopAlignHandguardIK;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Notify", meta = (AllowPrivateAccess = "true"))
		bool IsPostDeath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Notify", meta = (AllowPrivateAccess = "true"))
		bool IsRevived;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Notify", meta = (AllowPrivateAccess = "true"))
		bool EndCombat;
};

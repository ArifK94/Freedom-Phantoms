// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "WeaponAnimNotify.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class AnimType : uint8
{
	GrabWeapon UMETA(DisplayName = "GrabWeapon"),
	EndEquip UMETA(DisplayName = "EndEquip"),
	UnEquip UMETA(DisplayName = "UnEquip"),
	SwapWeapon UMETA(DisplayName = "SwapWeapon"),
	ReloadClipIn UMETA(DisplayName = "ReloadClipIn"),
	ReloadClipOut UMETA(DisplayName = "ReloadClipOut"),
	ReloadEnd UMETA(DisplayName = "ReloadEnd"),
	GrabClip UMETA(DisplayName = "GrabClip"),
	FireWeapon UMETA(DisplayName = "FireWeapon"),
	StopFire UMETA(DisplayName = "StopFire"),

};
UCLASS()
class FREEDOMPHANTOMS_API UWeaponAnimNotify : public UAnimNotify
{
	GENERATED_BODY()


		virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;


private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Notify", meta = (AllowPrivateAccess = "true"))
		AnimType animType;

	
};

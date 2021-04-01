// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MountedGun.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"

AMountedGun::AMountedGun()
{
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	CameraPositionSocket = "CamPos";
}

void AMountedGun::BeginPlay()
{
	Super::BeginPlay();

	FollowCamera->AttachToComponent(getMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CameraPositionSocket);
}


void AMountedGun::AddControllerPitchInput(float Val)
{
	RotationInput.Pitch = FMath::ClampAngle(RotationInput.Pitch + Val, PitchMin, PitchMax);
	RotationInput.Pitch = FRotator::ClampAxis(RotationInput.Pitch);

	FollowCamera->SetRelativeRotation(RotationInput);
}

void AMountedGun::AddControllerYawInput(float Val)
{
	RotationInput.Yaw = FMath::ClampAngle(RotationInput.Yaw + Val, YawMin, YawMax);
	RotationInput.Yaw = FRotator::ClampAxis(RotationInput.Yaw);

	FollowCamera->SetRelativeRotation(RotationInput);
}

void AMountedGun::SetPlayerControl(APlayerController* OurPlayerController)
{
	OurPlayerController->SetViewTargetWithBlend(this, .5f);
	SetComponentEyeViewPoint(FollowCamera);
}

void AMountedGun::RemovePlayerControl(APlayerController* OurPlayerController, class ACharacter* Character)
{
	OurPlayerController->SetViewTargetWithBlend(Character, .2f);
	StopFire();
}
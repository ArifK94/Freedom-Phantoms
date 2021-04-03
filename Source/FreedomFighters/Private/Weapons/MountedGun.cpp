// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MountedGun.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"

AMountedGun::AMountedGun()
{
	PrimaryActorTick.bCanEverTick = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	CameraPositionSocket = "CamPos";
}

void AMountedGun::SetIsAiming(bool isAiming)
{
	Super::SetIsAiming(isAiming);

	// clear Zoom timers if running
	GetWorldTimerManager().ClearTimer(THandler_ZoomFOVIn);
	GetWorldTimerManager().ClearTimer(THandler_ZoomFOVOut);

	TargetFOV = isAiming ? ZoomFOV : DefaultFOV;

	if (isAiming)
	{
		GetWorldTimerManager().SetTimer(THandler_ZoomFOVIn, this, &AMountedGun::ZoomIn, .01f, true);
	}
	else
	{
		GetWorldTimerManager().SetTimer(THandler_ZoomFOVOut, this, &AMountedGun::ZoomOut, .01f, true);
	}

}

void AMountedGun::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = FollowCamera->FieldOfView;
	TargetFOV = 0.0f;

	FollowCamera->AttachToComponent(getMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CameraPositionSocket);
	
	// So that the weapon fires from the FollowCamera view
	SetComponentEyeViewPoint(FollowCamera);
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
	OurPlayerController->SetViewTargetWithBlend(this, .2f);
}

void AMountedGun::RemovePlayerControl(APlayerController* OurPlayerController, class ACharacter* Character)
{
	OurPlayerController->SetViewTargetWithBlend(Character, .2f);
	StopFire();
}

void AMountedGun::ZoomIn()
{
	if (FollowCamera->FieldOfView > TargetFOV)
	{
		FollowCamera->FieldOfView--;
	}
	else
	{
		FollowCamera->SetFieldOfView(TargetFOV);
		GetWorldTimerManager().ClearTimer(THandler_ZoomFOVIn);
	}
}

void AMountedGun::ZoomOut()
{
	if (FollowCamera->FieldOfView < TargetFOV)
	{
		FollowCamera->FieldOfView++;
	}
	else
	{
		FollowCamera->SetFieldOfView(TargetFOV);
		GetWorldTimerManager().ClearTimer(THandler_ZoomFOVOut);
	}
}
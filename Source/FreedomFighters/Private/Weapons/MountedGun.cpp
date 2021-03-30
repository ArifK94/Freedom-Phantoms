// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MountedGun.h"

AMountedGun::AMountedGun()
{

}

void AMountedGun::AddControllerPitchInput(float Val)
{
	RotationInput.Pitch = FMath::ClampAngle(RotationInput.Pitch + Val, PitchMin, PitchMax);
	RotationInput.Pitch = FRotator::ClampAxis(RotationInput.Pitch);

	SetActorRelativeRotation(RotationInput);
}
void AMountedGun::AddControllerYawInput(float Val)
{
	RotationInput.Yaw = FMath::ClampAngle(RotationInput.Yaw + Val, YawMin, YawMax);
	RotationInput.Yaw = FRotator::ClampAxis(RotationInput.Yaw);

	SetActorRelativeRotation(RotationInput);
}
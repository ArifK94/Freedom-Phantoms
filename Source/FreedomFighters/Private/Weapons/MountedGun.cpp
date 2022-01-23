// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MountedGun.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CombatCharacter.h"
#include "Controllers/CustomPlayerController.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"


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

AMountedGun::AMountedGun()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp->SetCollisionProfileName(TEXT("MountedWeapon"));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	CameraPositionSocket = "CamPos";
	CharacterPositionSocket = "CharacterPosition";

	AdjustBehindMG = true;
	CanTraceInteraction = true;
	CanExit = true;

	StepBackAmount = 50.0f;
}

void AMountedGun::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = FollowCamera->FieldOfView;
	TargetFOV = 0.0f;

	FollowCamera->AttachToComponent(getMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CameraPositionSocket);
	
	// So that the weapon fires from the FollowCamera view
	SetComponentEyeViewPoint(FollowCamera);

	ResetCamera();
}

FString AMountedGun::OnInteractionFound_Implementation()
{
	return PickupMessage.ToString();
}


bool AMountedGun::CanInteract_Implementation(APawn* InPawn, AController* InController)
{
	if (CanTraceInteraction && GetOwner() == nullptr) {
		return true;
	}

	return false;
}

AActor* AMountedGun::OnPickup_Implementation(APawn* InPawn, AController* InController)
{
	if (!InPawn || !InController) {
		return nullptr;
	}

	auto CombatCharacter = Cast<ACombatCharacter>(InPawn);
	auto PlayerController = Cast<ACustomPlayerController>(InController);

	if (!CombatCharacter || !PlayerController) {
		return nullptr;
	}

	// Stop using the mounted gun if currently using it
	if (CombatCharacter->GetCurrentWeapon() == this)
	{
		if (CanExit)
		{
			PlayerController->DropMountedGun();
			CombatCharacter->DropMountedGun();
			RemovePlayerControl(PlayerController, CombatCharacter);

			// renable character input
			CombatCharacter->EnableInput(PlayerController);
			return nullptr;
		}
	}
	else
	{
		CombatCharacter->SetMountedGun(this);
		CombatCharacter->UseMountedGun();
		PlayerController->UseMountedGun();
		SetPlayerControl(PlayerController, CombatCharacter);
		return this;
	}

	return nullptr;
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

void AMountedGun::SetRotatioInput(FRotator Rotation)
{
	RotationInput = Rotation;

	FollowCamera->SetWorldRotation(RotationInput);
}

void AMountedGun::SetPlayerControl(APlayerController* OurPlayerController, ACharacter* Character)
{
	OurPlayerController->SetViewTargetWithBlend(this, .2f);
	PotentialOwner = nullptr;
}

void AMountedGun::RemovePlayerControl(APlayerController* OurPlayerController, ACharacter* Character)
{
	OurPlayerController->SetViewTargetWithBlend(Character, 0.0f);
	DropWeapon(true);
	PotentialOwner = nullptr;
}

void AMountedGun::DropWeapon(bool RemoveOwner, bool SimulatePhysics)
{
	AActor* MyOwner = GetOwner();


	// Get owner to step back behind the mounted gun
	if (MyOwner)
	{
		FVector NegativeVector =  UKismetMathLibrary::NegateVector(MyOwner->GetActorForwardVector());
		FVector TargetLocation = (NegativeVector * StepBackAmount) + MyOwner->GetActorLocation();
		MyOwner->SetActorLocation(TargetLocation);
	}

	if (RemoveOwner)
	{
		SetOwner(nullptr);
	}
	PotentialOwner = nullptr;

	SetIsAiming(false);
	StopFire();

	ResetCamera();
}

void AMountedGun::ResetCamera()
{
	RotationInput = FRotator::ZeroRotator;
	FollowCamera->SetRelativeRotation(RotationInput);
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


FVector AMountedGun::GetCharacterStandPos()
{
	return MeshComp->GetSocketLocation(CharacterPositionSocket);
}

FRotator AMountedGun::GetCharacterStandRot()
{
	return MeshComp->GetSocketRotation(CharacterPositionSocket);
}
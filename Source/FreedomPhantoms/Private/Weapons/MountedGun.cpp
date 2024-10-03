// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MountedGun.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CombatCharacter.h"
#include "Controllers/CustomPlayerController.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

AMountedGun::AMountedGun()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp->SetCollisionProfileName(TEXT("MountedWeapon"));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(MeshComp);

	CameraPositionSocket = "CamPos";
	CharacterPositionSocket = "CharacterPosition";

	AdjustBehindMG = true;
	CanTraceInteraction = true;
	CanExit = true;
	ClampPitch = true;
	ClampYaw = true;

	StepBackAmount = 50.0f;

	CrosshairErrorTolerance = .0f;


	weaponType = WeaponType::MountedGun;
}

void AMountedGun::SetIsAiming(bool isAiming)
{
	Super::SetIsAiming(isAiming);

	if (!GetWorld()) {
		return;
	}

	// clear Zoom timers if running
	if (THandler_ZoomFOVIn.IsValid()) {
		GetWorldTimerManager().ClearTimer(THandler_ZoomFOVIn);
	}

	if (THandler_ZoomFOVOut.IsValid()) {
		GetWorldTimerManager().ClearTimer(THandler_ZoomFOVOut);
	}

	TargetFOV = isAiming ? ZoomFOV : DefaultFOV;

	if (isAiming)
	{
		if (!THandler_ZoomFOVIn.IsValid()) {
			GetWorldTimerManager().SetTimer(THandler_ZoomFOVIn, this, &AMountedGun::ZoomIn, .01f, true);
		}
	}
	else
	{
		if (!THandler_ZoomFOVOut.IsValid()) {
			GetWorldTimerManager().SetTimer(THandler_ZoomFOVOut, this, &AMountedGun::ZoomOut, .01f, true);
		}
	}

}

void AMountedGun::BeginPlay()
{
	Super::BeginPlay();

	MeshComp->SetGenerateOverlapEvents(true);
	MeshComp->SetCollisionProfileName(TEXT("MountedWeapon"));

	DefaultFOV = FollowCamera->FieldOfView;
	TargetFOV = 0.0f;

	FollowCamera->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CameraPositionSocket);

	// So that the weapon fires from the FollowCamera view
	SetComponentEyeViewPoint(FollowCamera);

	ResetCamera();
}

void AMountedGun::DelayedInit()
{
	Super::DelayedInit();

	if (UseParentMuzzle || UseParentCameraPositionSocket)
	{
		FollowCamera->AttachToComponent(GetParentMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CameraPositionSocket);
	}
}

FString AMountedGun::OnInteractionFound_Implementation(APawn* InPawn, AController* InController)
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
	if (ClampPitch) {
		RotationInput.Pitch = FMath::Clamp(RotationInput.Pitch + Val, PitchMin, PitchMax);
	}
	else {
		RotationInput.Pitch += Val;
	}
}

void AMountedGun::AddControllerYawInput(float Val)
{
	if (ClampYaw) {
		RotationInput.Yaw = FMath::Clamp(RotationInput.Yaw + Val, YawMin, YawMax);
	}
	else {
		RotationInput.Yaw += Val;
	}
}

void AMountedGun::SetRotationInput(FRotator TargetRotation)
{
	RotationInput = TargetRotation;
}

void AMountedGun::SetRotationInput(FRotator TargetRotation, float LerpSpeed)
{
	if (!GetWorld()) {
		return;
	}

	RotationInput = UKismetMathLibrary::RInterpTo(RotationInput, TargetRotation, GetWorld()->DeltaTimeSeconds, LerpSpeed);
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
		FVector NegativeVector = UKismetMathLibrary::NegateVector(MyOwner->GetActorForwardVector());
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
}

void AMountedGun::ZoomIn()
{
	if (!GetWorld()) {
		return;
	}

	if (FollowCamera->FieldOfView > TargetFOV)
	{
		FollowCamera->FieldOfView--;
	}
	else
	{
		FollowCamera->SetFieldOfView(TargetFOV);

		if (THandler_ZoomFOVIn.IsValid()) {
			GetWorldTimerManager().ClearTimer(THandler_ZoomFOVIn);
		}
	}
}

void AMountedGun::ZoomOut()
{
	if (!GetWorld()) {
		return;
	}

	if (FollowCamera->FieldOfView < TargetFOV)
	{
		FollowCamera->FieldOfView++;
	}
	else
	{
		FollowCamera->SetFieldOfView(TargetFOV);

		if (THandler_ZoomFOVOut.IsValid()) {
			GetWorldTimerManager().ClearTimer(THandler_ZoomFOVOut);
		}
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

FTransform AMountedGun::GetMGBaseTransform()
{
	USkeletalMeshComponent* MGMesh = Cast<USkeletalMeshComponent>(GetParentMesh());
	return MGMesh->GetSocketTransform(TurretBaseBoneName);
}
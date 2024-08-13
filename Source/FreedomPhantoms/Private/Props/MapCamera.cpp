#include "Props/MapCamera.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"

AMapCamera::AMapCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	SetTickableWhenPaused(true);

	SceneCaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
	RootComponent = SceneCaptureComponent2D;
	SceneCaptureComponent2D->SetTickableWhenPaused(true);
	SceneCaptureComponent2D->bCaptureEveryFrame = false;

	PanningSpeedMultiplier = 150.0f;
	ZoomSpeedMultiplier = 150.0f;

	ZoomMin = 0.0f;
	ZoomMax = 0.0f;

	LockInput = false;
}

void AMapCamera::BeginPlay()
{
	Super::BeginPlay();

	DefaultLocation = GetActorLocation();
	LocationInput = DefaultLocation;

	Deactivate();
}

void AMapCamera::Tick(float DelatTime)
{
	Super::Tick(DelatTime);

	if (StartPostTimer)
	{
		if (CurrentPostActivateTimer > 0)
		{
			CurrentPostActivateTimer--;
		}
		else
		{
			StartPostTimer = false;
			CurrentPostActivateTimer = 0.f;

			PostActivate();
		}
	}

	// Run this code only if game is paused.
	if (UGameplayStatics::IsGamePaused(GetWorld()))
	{
		// Should the zoom value be corrected to avoid go through actors?
		if (ZoomCorrectionZ != .0f && !UKismetMathLibrary::NearlyEqual_FloatFloat(LocationInput.Z, ZoomCorrectionZ))
		{
			LocationInput.Z = UKismetMathLibrary::Lerp(GetActorLocation().Z, ZoomCorrectionZ, DelatTime * 8.f);
			SetActorLocation(LocationInput);
		}
		// Should this actor's Up axis be corrected after correcting the zoom if it is out of zoom bounds?
		else if (GetActorLocation().Z < ZoomMin || GetActorLocation().Z > ZoomMax)
		{
			float ZClamp = FMath::Clamp(GetActorLocation().Z, ZoomMin, ZoomMax);

			LocationInput.Z = UKismetMathLibrary::Lerp(GetActorLocation().Z, ZClamp, DelatTime * 5.f);
			SetActorLocation(LocationInput);

			ZoomCorrectionZ = .0f;
		}
		// Is there no need for zoom correcting? 
		else
		{
			ZoomCorrectionZ = .0f;
		}
	}
}

void AMapCamera::Activate()
{
	StartPostTimer = true;
	CurrentPostActivateTimer = 2.f;

	SceneCaptureComponent2D->bCaptureEveryFrame = true;
	SetActorHiddenInGame(false);
}

void AMapCamera::Deactivate()
{
	StartPostTimer = false;
	CurrentPostActivateTimer = 0.f;

	SceneCaptureComponent2D->bCaptureEveryFrame = false;
	SetActorHiddenInGame(true);
}

void AMapCamera::PostActivate()
{
	// setting to true lowers the FPS and since this scene capture is displayed on the pause menu, this should be false
	SceneCaptureComponent2D->bCaptureEveryFrame = false;
}

void AMapCamera::MoveToPlayerLocation()
{
	auto Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	if (Player == nullptr) {
		return;
	}

	FVector TargetLocation = FVector(Player->GetActorLocation().X, Player->GetActorLocation().Y, GetActorLocation().Z);
	SetActorLocation(TargetLocation);

	LocationInput = GetActorLocation();
}

void AMapCamera::MoveForward(float Value)
{
	if (LockInput) {
		return;
	}

	if (Value == 0.0f) {
		return;
	}

	float Amount = PanningSpeedMultiplier;
	if (Value < 0.0f)
	{
		Amount *= -1.0f;
	}

	// The bounds of X are based on the camera default location as the camera is placed and bounds were set based on this location in the editor.
	float Min = (DefaultLocation.X + PanningMin.X) - (DefaultLocation.Z / GetActorLocation().Z);
	float Max = (DefaultLocation.X + PanningMax.X) + (DefaultLocation.Z / GetActorLocation().Z);

	LocationInput.X = FMath::Clamp(GetActorLocation().X + Amount, Min, Max);
	SetActorLocation(LocationInput);

	CheckCollision();
}

void AMapCamera::MoveRight(float Value)
{
	if (LockInput) {
		return;
	}

	if (Value == 0.0f) {
		return;
	}

	float Amount = PanningSpeedMultiplier;
	if (Value < 0.0f)
	{
		Amount *= -1.0f;
	}

	// The bounds of Y are based on the camera default location as the camera is placed and bounds were set based on this location in the editor.
	float Min = (DefaultLocation.Y + PanningMin.Y) - (DefaultLocation.Z / GetActorLocation().Z);
	float Max = (DefaultLocation.Y + PanningMax.Y) + (DefaultLocation.Z / GetActorLocation().Z);

	LocationInput.Y = FMath::Clamp(GetActorLocation().Y + Amount, Min, Max);
	SetActorLocation(LocationInput);

	CheckCollision();
}

void AMapCamera::Zoom(float Value)
{
	if (LockInput) {
		return;
	}

	if (Value == 0.0f) {
		return;
	}

	float Amount = ZoomSpeedMultiplier;
	if (Value < 0.0f)
	{
		Amount *= -1.0f;
	}

	ZoomCorrectionZ = .0f;

	LocationInput.Z = FMath::Clamp(GetActorLocation().Z + Amount, ZoomMin, ZoomMax);
	SetActorLocation(LocationInput);


	// Update the location limits, add a positive then a negative since the camera can be at the limit for x or y
	MoveForward(1.0f);
	MoveForward(-1.0f);
	MoveRight(1.0f);
	MoveRight(-1.0f);

	CheckCollision();
}

/**
* Check if the camera has hit an actor in multiple directions.
*/
void AMapCamera::CheckCollision()
{
	float Length = 500.f;

	FVector Start = GetActorLocation();

	FVector CenterEnd = (GetActorForwardVector() * Length) + Start;

	FVector ForwardEnd = (GetActorUpVector() * Length) + Start;
	FVector BackEnd = (-GetActorUpVector() * Length) + Start;

	FVector LeftEnd = (-GetActorRightVector() * Length) + Start;
	FVector RightEnd = (GetActorRightVector() * Length) + Start;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;

	FHitResult OutHitCenter;
	if (GetWorld()->LineTraceSingleByObjectType(OutHitCenter, Start, ForwardEnd, ObjectParams))
	{
		if (OutHitCenter.bBlockingHit)
		{
			ZoomCorrectionZ = GetActorLocation().Z + OutHitCenter.ImpactPoint.Z;
		}
	}

	FHitResult OutHitForward;
	if (GetWorld()->LineTraceSingleByObjectType(OutHitForward, Start, ForwardEnd, ObjectParams))
	{
		if (OutHitForward.bBlockingHit)
		{
			ZoomCorrectionZ = GetActorLocation().Z + OutHitForward.ImpactPoint.Z;
		}
	}

	FHitResult OutHitBack;
	if (GetWorld()->LineTraceSingleByObjectType(OutHitBack, Start, BackEnd, ObjectParams))
	{
		if (OutHitBack.bBlockingHit)
		{
			ZoomCorrectionZ = GetActorLocation().Z + OutHitBack.ImpactPoint.Z;
		}
	}

	FHitResult OutHitLeft;
	if (GetWorld()->LineTraceSingleByObjectType(OutHitLeft, Start, LeftEnd, ObjectParams))
	{
		if (OutHitLeft.bBlockingHit)
		{
			ZoomCorrectionZ = GetActorLocation().Z + OutHitLeft.ImpactPoint.Z;
		}
	}

	FHitResult OutHitRight;
	if (GetWorld()->LineTraceSingleByObjectType(OutHitRight, Start, RightEnd, ObjectParams))
	{
		if (OutHitRight.bBlockingHit)
		{
			ZoomCorrectionZ = GetActorLocation().Z + OutHitRight.ImpactPoint.Z;
		}
	}
}

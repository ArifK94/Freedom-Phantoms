#include "Props/MapCamera.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"
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

	LocationInput.Z = FMath::Clamp(GetActorLocation().Z + Amount, ZoomMin, ZoomMax);
	SetActorLocation(LocationInput);


	// Updaye the location limits, add a positive then a negative since the camera can be at the limit for x or y
	MoveForward(1.0f);
	MoveForward(-1.0f);
	MoveRight(1.0f);
	MoveRight(-1.0f);
}
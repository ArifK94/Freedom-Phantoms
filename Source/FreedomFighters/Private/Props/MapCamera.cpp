#include "Props/MapCamera.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AMapCamera::AMapCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	SetTickableWhenPaused(true);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	RootComponent = Camera;

	SceneCaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
	SceneCaptureComponent2D->SetTickableWhenPaused(true);

	PostProcessor = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessor"));

	PanningSpeedMultiplier = 150.0f;
	ZoomSpeedMultiplier = 150.0f;

	PanningMinX = 0.0f;
	PanningMaxX = 0.0f;
	PanningMinY = 0.0f;
	PanningMaxY = 0.0f;

	ZoomMin = 0.0f;
	ZoomMax = 0.0f;
}

void AMapCamera::BeginPlay()
{
	Super::BeginPlay();

	DefaultLocation = GetActorLocation();
	LocationInput = DefaultLocation;

	//TArray<FEngineShowFlagsSetting> Test;

	//FEngineShowFlagsSetting EngineShowFlagsSetting = FEngineShowFlagsSetting();
	//EngineShowFlagsSetting.ShowFlagName = "SkeletalMeshes";
	//EngineShowFlagsSetting.Enabled = false;
	//Test.Add(EngineShowFlagsSetting);


	//FEngineShowFlagsSetting EngineShowFlagsSetting2 = FEngineShowFlagsSetting();
	//EngineShowFlagsSetting2.ShowFlagName = "Landscape";
	//EngineShowFlagsSetting2.Enabled = false;
	//Test.Add(EngineShowFlagsSetting2);

	//SceneCaptureComponent2D->ShowFlagSettings = Test;
}

void AMapCamera::MoveToPlayerLocation()
{
	auto Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	FVector TargetLocation = FVector(Player->GetActorLocation().X, Player->GetActorLocation().Y, GetActorLocation().Z);
	SetActorLocation(TargetLocation);

	LocationInput = GetActorLocation();
}

void AMapCamera::MoveForward(float Value)
{
	if (Value == 0.0f) {
		return;
	}

	float Amount = PanningSpeedMultiplier;
	if (Value < 0.0f)
	{
		Amount *= -1.0f;
	}

	float Min = PanningMinX - GetActorLocation().Z;
	float Max = PanningMaxX + GetActorLocation().Z;

	LocationInput.X = FMath::Clamp(GetActorLocation().X + Amount, Min, Max);
	SetActorLocation(LocationInput);
}

void AMapCamera::MoveRight(float Value)
{
	if (Value == 0.0f) {
		return;
	}

	float Amount = PanningSpeedMultiplier;
	if (Value < 0.0f)
	{
		Amount *= -1.0f;
	}

	float Min = PanningMinY - GetActorLocation().Z;
	float Max = PanningMaxY + GetActorLocation().Z;

	LocationInput.Y = FMath::Clamp(GetActorLocation().Y + Amount, Min, Max);
	SetActorLocation(LocationInput);
}

void AMapCamera::Zoom(float Value)
{
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
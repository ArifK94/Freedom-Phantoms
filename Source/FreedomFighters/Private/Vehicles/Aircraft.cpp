#include "Vehicles/Aircraft.h"

#include "Characters/BaseCharacter.h"

#include "Props/AircraftSplinePath.h"
#include "Props/TargetSystemMarker.h"

#include "Weapons/Weapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SplineComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"
#include "CustomComponents/HealthComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"


AAircraft::AAircraft()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Root"));
	RootComponent = CapsuleComponent;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("Vehicle"));
	Mesh->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::KeepRelativeTransform);

	EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	EngineAudio->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);

	PilotAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("PilotAudio"));
	PilotAudio->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	ThermalVisionPPComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("ThermalVisionPPComp"));
	ThermalVisionPPComp->AttachToComponent(FollowCamera, FAttachmentTransformRules::KeepRelativeTransform);

	WingRotationSpeed = 100000.0;

	PathFollowDuration = 10.0f;
	TotalLaps = 1;

	CurrentWeaponIndex = 0;
}

void AAircraft::AddUIWidget()
{
	UWorld* World = GetWorld();

	if (!World) return;

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(World, HUDWidgetClass);

		if (HUDWidget)
		{
			UWidgetLayoutLibrary::RemoveAllWidgets(World);
			HUDWidget->AddToViewport();
		}
	}
}

void AAircraft::RemoveUIWidget()
{
	UWorld* World = GetWorld();

	if (!World) return;

	if (HUDWidget)
	{
		UWidgetLayoutLibrary::RemoveAllWidgets(World);
		HUDWidget = nullptr;
	}
}


void AAircraft::BeginPlay()
{
	Super::BeginPlay();

	ThermalVisionPPComp->bEnabled = false;
	CreateThermalMatInstances();

	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AAircraft::OnOverlapBegin);
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AAircraft::OnOverlapBegin);

	FindPath();

	if (AircraftWeapons.Num() > 0) {
		SpawnWeapon();
	}

}

void AAircraft::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurveTimeline.TickTimeline(DeltaTime);

	CurrentWingSpeed = WingRotationSpeed * DeltaTime;

	SetTargetSystem();
}

void AAircraft::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AircraftPath != nullptr)
	{
		FVector CollisionLocation = GetActorLocation();
		AAircraftSplinePath* CollidedPath = Cast<AAircraftSplinePath>(OtherActor);

		if (CollidedPath)
		{
			FVehicleSplinePoint CurrentSplinePoint = CollidedPath->GetVehicleSplinePoint(CollisionLocation);

			if (CurrentSplinePoint.PointIndex != -1)
			{

				switch (CurrentSplinePoint.MovementType)
				{
				case AircraftSplineMovement::Throttling:
					CurrentAircraftMovement = AircraftMovement::MovingForward;
					break;
				case AircraftSplineMovement::Hovering:
					CurrentAircraftMovement = AircraftMovement::Hovering;
					CurveTimeline.Stop();
					break;
				case AircraftSplineMovement::Stopping:
					CurrentAircraftMovement = AircraftMovement::Stopping;
					break;
				default:
					CurrentAircraftMovement = AircraftMovement::Grounded;
					break;
				}
			}
		}
	}
}

void AAircraft::FindPath()
{
	TArray<AActor*> TargetActor;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), AircraftPathTagName, TargetActor);

	if (AircraftPath == nullptr)
	{
		for (AActor* Actor : TargetActor)
		{
			AircraftPath = Cast<AAircraftSplinePath>(Actor);

			if (AircraftPath != nullptr)
			{

				// setup time line for following the path
				if (CurveFloat)
				{
					FOnTimelineFloat TimelineProgress;
					TimelineProgress.BindUFunction(this, FName("FollowSplinePath"));
					CurveTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
					CurveTimeline.SetLooping(false);
					CurveTimeline.SetPlayRate(1.0f / PathFollowDuration);
					CurveTimeline.PlayFromStart();
				}
				break;
			}
		}
	}
}

void AAircraft::FollowSplinePath(float Value)
{
	USplineComponent* SplinePathComp = AircraftPath->GetSplinePathComp();
	float Alpha = UKismetMathLibrary::Lerp(0.0f, SplinePathComp->GetSplineLength(), Value);

	FVector TargetLocation = SplinePathComp->GetLocationAtDistanceAlongSpline(Alpha, ESplineCoordinateSpace::World);
	FRotator TargetRotation = SplinePathComp->GetRotationAtDistanceAlongSpline(Alpha, ESplineCoordinateSpace::World);

	SetActorLocationAndRotation(TargetLocation, TargetRotation);


	// if reached the end
	if (Alpha >= SplinePathComp->GetSplineLength())
	{
		if (CurrentLap < TotalLaps) // restart the lap if laps remaining
		{
			CurrentLap++;
			CurveTimeline.PlayFromStart();
		}
		else
		{
			OnDestroy();
		}
	}

}

void AAircraft::SpawnWeapon()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (FAircraftWeapon AircraftWeapon : AircraftWeapons)
	{
		AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(AircraftWeapon.Weapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (Weapon)
		{
			Weapon->SetComponentEyeViewPoint(FollowCamera);
			Weapon->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AircraftWeapon.WeaponSocketName);
			Weapon->GetClipAudioComponent()->AttachToComponent(FollowCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			Weapon->GetShotAudioComponent()->AttachToComponent(FollowCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

			WeaponObjs.Add(Weapon);
		}
	}

	UpdateWeaponView();
}

void AAircraft::ChangeWeapon()
{
	// increment the index if current index is less than the array of weapons
	// otherwise go back to the first index
	if (CurrentWeaponIndex < AircraftWeapons.Num() - 1)
	{
		CurrentWeaponIndex++;
	}
	else
	{
		CurrentWeaponIndex = 0;
	}

	if (CurrentWeaponObj) {
		CurrentWeaponObj->StopFire(); // stop firing current weapon before switching to another
	}

	UpdateWeaponView();
}

void AAircraft::UpdateWeaponView()
{
	CurrentAircraftWeapon = AircraftWeapons[CurrentWeaponIndex]; // set the current weapon to first weapon by default
	CurrentWeaponObj = WeaponObjs[CurrentWeaponIndex];

	FollowCamera->AttachToComponent(CurrentWeaponObj->getMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CurrentWeaponObj->GetMuzzleSocket()); // Attach to weapon muzzle
	FollowCamera->SetRelativeRotation(RotationInput); // set the follow camera to rotation input

	FollowCamera->SetFieldOfView(CurrentWeaponObj->GetZoomFOV());
	OnUpdateWeaponUI.Broadcast(this);
}

void AAircraft::AddControllerPitchInput(float Val)
{
	RotationInput.Pitch = FMath::ClampAngle(RotationInput.Pitch + Val, CurrentAircraftWeapon.PitchMin, CurrentAircraftWeapon.PitchMax);
	RotationInput.Pitch = FRotator::ClampAxis(RotationInput.Pitch);

	FollowCamera->SetRelativeRotation(RotationInput);
}
void AAircraft::AddControllerYawInput(float Val)
{
	RotationInput.Yaw = FMath::ClampAngle(RotationInput.Yaw + Val, CurrentAircraftWeapon.YawMin, CurrentAircraftWeapon.YawMax);
	RotationInput.Yaw = FRotator::ClampAxis(RotationInput.Yaw);

	FollowCamera->SetRelativeRotation(RotationInput);
}

void AAircraft::ShowOutlines(bool CanShow)
{
	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), Characters);

	for (AActor* Actor : Characters)
	{
		ABaseCharacter* Character = Cast<ABaseCharacter>(Actor);

		if (Character != nullptr)
		{
			Character->ShowCharacterOutline(CanShow);
		}
	}
}

void AAircraft::SetTargetSystem()
{
	AActor* MyOwner = GetOwner();

	if (!MyOwner) {
		return;
	}

	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), Characters);

	// adding characters to targetting
	for (AActor* Actor : Characters)
	{
		ABaseCharacter* Character = Cast<ABaseCharacter>(Actor);

		if (Character != nullptr)
		{
			UHealthComponent* CurrentHealth = Cast<UHealthComponent>(Character->GetComponentByClass(UHealthComponent::StaticClass()));

			if (CurrentHealth)
			{
				bool isFriendly = UHealthComponent::IsFriendly(MyOwner, Character);

				if (!isFriendly && CurrentHealth->IsAlive())
				{
					if (!IfNodeExists(Character))
					{
						FTargetSystemNode* TargetNode = new FTargetSystemNode;
						TargetNode->Character = Character;
						TargetNode->Marker = nullptr;
						TargetSystemNodes.Add(TargetNode);
					}
				}
			}
		}
	}



	if (TargetSystemNodes.Num() > 0)
	{
		for (int i = 0; i < TargetSystemNodes.Num(); i++)
		{
			FTargetSystemNode* TargetNode = TargetSystemNodes[i];

			// add or update target marker based on character location
			FActorSpawnParameters SpawnParams;

			if (TargetNode->Marker == nullptr)
			{
				if (TargetMarkerClass) {
					TargetNode->Marker = GetWorld()->SpawnActor<ATargetSystemMarker>(TargetMarkerClass, TargetNode->Character->GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
				}
			}
			else
			{
				TargetNode->Marker->SetActorLocation(TargetNode->Character->GetActorLocation());
			}


			// remove dead characters from the targetting system
			UHealthComponent* CurrentHealth = Cast<UHealthComponent>(TargetNode->Character->GetComponentByClass(UHealthComponent::StaticClass()));

			if (CurrentHealth)
			{
				if (!CurrentHealth->IsAlive())
				{
					if (TargetNode->Marker) {
						TargetNode->Marker->Destroy();
					}
					TargetSystemNodes.RemoveAt(i);
				}
			}
		}
	}

}

bool AAircraft::IfNodeExists(AActor* TargetActor)
{
	for (FTargetSystemNode* node : TargetSystemNodes)
	{
		if (node->Character == TargetActor)
			return true;
	}

	return false;
}


void AAircraft::CreateThermalMatInstances()
{
	for (int i = 0; i < ThermalMaterials.Num(); i++)
	{
		UMaterialInstanceDynamic* MaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), ThermalMaterials[i]);
		ThermalVisionPPComp->AddOrUpdateBlendable(MaterialInstance, 0.0f); // set all to invisible
		ThermalMaterialInstances.Add(MaterialInstance);
	}
}

void AAircraft::UpdateCurrentThermalVision(float InWeight)
{
	ThermalVisionPPComp->AddOrUpdateBlendable(ThermalMaterialInstances[CurrentThermalMatIndex], InWeight);
}

void AAircraft::ChangeThermalVision()
{
	if (ThermalMaterials.Num() <= 0) {
		return;
	}

	UpdateCurrentThermalVision(0.0f); // disable current material

	if (CurrentThermalMatIndex < ThermalMaterialInstances.Num() - 1)
	{
		CurrentThermalMatIndex++;
	}
	else
	{
		CurrentThermalMatIndex = 0;
	}

	UpdateCurrentThermalVision(1.0f);
}


void AAircraft::SetPlayerControl(APlayerController* OurPlayerController)
{
	OurPlayerController->SetViewTargetWithBlend(this, .5f);

	ThermalVisionPPComp->bEnabled = true;
	ShowOutlines(true);
	UpdateCurrentThermalVision(1.0f);
	AddUIWidget();
}

void AAircraft::OnDestroy()
{
	RemoveUIWidget();
	ShowOutlines(false);

	OnAircraftDestroy.Broadcast(this);


	// desttroy all attached actors to this aircraft
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	for (AActor* ChildActor : AttachedActors)
	{
		ChildActor->Destroy();
	}

	Destroy();

}
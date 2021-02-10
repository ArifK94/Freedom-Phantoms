#include "Vehicles/Aircraft.h"
#include "Props/AircraftSplinePath.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Weapons/Weapon.h"


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

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	WingRotationSpeed = 100000.0;

	PathFollowDuration = 10.0f;
	TotalLaps = 1;

	CurrentWeaponIndex = 0;
}

void AAircraft::BeginPlay()
{
	Super::BeginPlay();

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

				/// destroy the helicopter once reached the last path point
	/*			if (CurrentSplinePoint.PointIndex >= CollidedPath->GetSplinePathComp()->GetNumberOfSplinePoints() - 1)
				{
					Destroy();
				}*/
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
}

void AAircraft::SpawnWeapon()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (FAircraftWeapon AircraftWeapon : AircraftWeapons)
	{
		AircraftWeapon.WeaponObj = GetWorld()->SpawnActor<AWeapon>(AircraftWeapon.Weapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (AircraftWeapon.WeaponObj)
		{
			AircraftWeapon.WeaponObj->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AircraftWeapon.WeaponSocketName);
			WeaponObjs.Add(AircraftWeapon.WeaponObj);
		}
	}

	CurrentAircraftWeapon = AircraftWeapons[CurrentWeaponIndex];
	CurrentWeaponObj = WeaponObjs[CurrentWeaponIndex]; // set the current weapon to first weapon by default
	FollowCamera->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CurrentAircraftWeapon.CameraSocketName);
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


	CurrentAircraftWeapon = AircraftWeapons[CurrentWeaponIndex]; // set the current weapon to first weapon by default
	CurrentWeaponObj = WeaponObjs[CurrentWeaponIndex];
	//FollowCamera->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CurrentAircraftWeapon.CameraSocketName);

	OnUpdateWeaponUI.Broadcast(this);
}

void AAircraft::SetPlayerControl(APlayerController* OurPlayerController)
{
	OurPlayerController->SetViewTargetWithBlend(this, .5f);
}

void AAircraft::AddControllerPitchInput(float Val)
{
	RotationInput.Pitch += Val;
	FollowCamera->SetRelativeRotation(RotationInput);
}
void AAircraft::AddControllerYawInput(float Val)
{
	RotationInput.Yaw += Val;
	FollowCamera->SetRelativeRotation(RotationInput);
}
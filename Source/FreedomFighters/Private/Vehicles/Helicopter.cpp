// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/Helicopter.h"
#include "Props/AircraftSplinePath.h"
#include "Accessories/Rope.h"

#include "Characters/BaseCharacter.h"
#include "Characters/CombatCharacter.h"

#include "Camera/CameraComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/SplineComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/EngineTypes.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

AHelicopter::AHelicopter()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	HelicopterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelicopterMesh"));
	HelicopterMesh->SetCollisionProfileName(TEXT("Vehicle"));
	HelicopterMesh->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);


	RotorAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("RotorAudio"));
	RotorAudio->AttachToComponent(HelicopterMesh, FAttachmentTransformRules::KeepRelativeTransform);

	PathFollowDuration = 10.0f;
}


void AHelicopter::BeginPlay()
{
	Super::BeginPlay();

	SpawnPassenger();

	FindPath();

	HelicopterMesh->OnComponentBeginOverlap.AddDynamic(this, &AHelicopter::OnOverlapBegin);
}

void AHelicopter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentDeltaTime = DeltaTime;
	CurveTimeline.TickTimeline(DeltaTime);

	WaitForRepelling();
}



void AHelicopter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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
				FVehicleSplinePoint NextSplinePoint = CollidedPath->GetNextSplinePoint(CurrentSplinePoint.PointIndex);

				switch (CurrentSplinePoint.MovementType)
				{
				case AircraftSplineMovement::Throttling:
					CurrentMovement = HelicopterMovement::MovingForward;
					break;
				case AircraftSplineMovement::Hovering:
					CurrentMovement = HelicopterMovement::Hovering;
					CurveTimeline.Stop();
					break;
				case AircraftSplineMovement::Stopping:
					CurrentMovement = HelicopterMovement::Stopping;
					break;
				default:
					CurrentMovement = HelicopterMovement::Grounded;
					break;
				}
			}
		}
	}
}

void AHelicopter::FindPath()
{
	TArray<AActor*> TargetActor;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), AircraftPathTagName, TargetActor);

	if (AircraftPath == nullptr)
	{
		for (AActor* Actor : TargetActor)
		{
			if (Cast<AAircraftSplinePath>(Actor))
			{
				AircraftPath = Cast<AAircraftSplinePath>(Actor);

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

void AHelicopter::FollowSplinePath(float Value)
{
	USplineComponent* SplinePathComp = AircraftPath->GetSplinePathComp();
	float Alpha = UKismetMathLibrary::Lerp(0.0f, SplinePathComp->GetSplineLength(), Value);

	FVector TargetLocation = SplinePathComp->GetLocationAtDistanceAlongSpline(Alpha, ESplineCoordinateSpace::World);
	FRotator TargetRotation = SplinePathComp->GetRotationAtDistanceAlongSpline(Alpha, ESplineCoordinateSpace::World);

	SetActorLocationAndRotation(TargetLocation, TargetRotation);

}

void AHelicopter::SpawnPassenger()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int i = 0; i < HelicopterSeating.Num(); i++)
	{
		FHelicopterSeating HeliSeat = HelicopterSeating[i];

		if (HeliSeat.Character)
		{
			HeliSeat.CharacterObj = GetWorld()->SpawnActor<ABaseCharacter>(HeliSeat.Character, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (HeliSeat.CharacterObj)
			{
				ABaseCharacter* Character = HeliSeat.CharacterObj;

				Character->GetCharacterMovement()->DefaultLandMovementMode = EMovementMode::MOVE_Flying;
				Character->AttachToComponent(HelicopterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeliSeat.SeatingSocketName);
				Character->SetActorRelativeLocation(FVector::ZeroVector);
				Character->SetActorRelativeRotation(FRotator::ZeroRotator);
				Character->SetIsInHelicopter(true);
				Character->SetHelicopterSeatPosition(HeliSeat.SeatPosition);
				HeliSeat.OwningHelicopter = this;



				Character->SetHelicopterSeating(HeliSeat);


				FRotator CamRotation = HeliSeat.CharacterObj->FollowCamera->GetComponentRotation();

				float TargetYaw = FMath::Clamp(CamRotation.Yaw, HeliSeat.CameraViewYawMin, HeliSeat.CameraViewYawMax);

				FRotator TargetRotation = UKismetMathLibrary::MakeRotator(CamRotation.Roll, CamRotation.Pitch, TargetYaw);
				//	HeliSeat.CharacterObj->FollowCamera->SetWorldRotation(TargetRotation);


				OccupiedSeating.Add(HeliSeat);
			}
		}
	}
}

void AHelicopter::WaitForRepelling()
{
	if (CurrentMovement == HelicopterMovement::Hovering && OccupiedSeating.Num() > 0)
	{
		CurveTimeline.Stop();

		if (RopeClass != nullptr)
		{
			if (leftRopeObj == nullptr)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				leftRopeObj = GetWorld()->SpawnActor<ARope>(RopeClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				leftRopeObj->AttachToComponent(HelicopterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftRopeSocket);
			}

			if (rightRopeObj == nullptr)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				rightRopeObj = GetWorld()->SpawnActor<ARope>(RopeClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				rightRopeObj->AttachToComponent(HelicopterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightRopeSocket);
			}

		}


		for (int i = 0; i < OccupiedSeating.Num(); i++)
		{

			if (OccupiedSeating[i].Role == HelicopterRole::SideGunner)
			{
				FHelicopterSeating Passenger = OccupiedSeating[i];

				if (!Passenger.CharacterObj->IsInHelicopter()) {
					OccupiedSeating.RemoveAt(i);
				}
				else
				{
					if (Passenger.isRopeLeftSide)
					{
						if (!isLeftRappelOccupied)
						{
							Passenger.CharacterObj->AttachToComponent(HelicopterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftRopeSocket);
							Passenger.CharacterObj->SetIsRepellingDown(true);
							//	isLeftRappelOccupied = true;
						}
					}
					else
					{
						if (!isRightRappelOccupied)
						{
							Passenger.CharacterObj->AttachToComponent(HelicopterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightRopeSocket);
							Passenger.CharacterObj->SetIsRepellingDown(true);
							//	isRightRappelOccupied = true;
						}
					}
				}
			}
		}
	}

	if (OccupiedSeating.Num() <= 0)
	{
		if (leftRopeObj) {
			leftRopeObj->DropRope();
		}

		if (rightRopeObj) {
			rightRopeObj->DropRope();
		}

		CurveTimeline.Play();

		CurrentMovement = HelicopterMovement::MovingForward;
	}
}

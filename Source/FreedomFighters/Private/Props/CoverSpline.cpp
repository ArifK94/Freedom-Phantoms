// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/CoverSpline.h"

#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "Engine/Engine.h"
#include "Engine/TriggerBox.h"
#include "Engine/StaticMesh.h"

#include "Components/BoxComponent.h"

#include <array>


ACoverSpline::ACoverSpline()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SplineComponent = CreateDefaultSubobject<USplineComponent>("Spline");
	if (SplineComponent)
	{
		SetRootComponent(SplineComponent);
		SplineComponent->SetCollisionProfileName(TEXT("OverlapAll"));
		SplineComponent->OnComponentBeginOverlap.AddDynamic(this, &ACoverSpline::OnCoverBeginOverlap);
	}
}

void ACoverSpline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (SplineComponent && SplineMeshMap.Num() > 0)
	{
		// lookup all pertinent values
		FSplineMeshDetails* StartMeshDetails = nullptr;
		if (SplineMeshMap.Contains(ESplineMeshType::START))
		{
			StartMeshDetails = SplineMeshMap.Find(ESplineMeshType::START);
		}

		FSplineMeshDetails* EndMeshDetails = nullptr;
		if (SplineMeshMap.Contains(ESplineMeshType::END))
		{
			EndMeshDetails = SplineMeshMap.Find(ESplineMeshType::END);
		}

		FSplineMeshDetails* DefaultMeshDetails = nullptr;
		if (SplineMeshMap.Contains(ESplineMeshType::DEFAULT))
		{
			DefaultMeshDetails = SplineMeshMap.Find(ESplineMeshType::DEFAULT);
		}
		else
		{
			// exit if we don't have a default mesh to work with
			return;
		}

		const int32 SplinePoints = SplineComponent->GetNumberOfSplinePoints();

		for (int SplineCount = 0; SplineCount < (SplinePoints); SplineCount++)
		{
			// define the positions of the points and tangents
			const FVector StartPoint = SplineComponent->GetLocationAtSplinePoint(SplineCount, ESplineCoordinateSpace::Type::Local);
			const FVector StartTangent = SplineComponent->GetTangentAtSplinePoint(SplineCount, ESplineCoordinateSpace::Type::Local);
			const FVector EndPoint = SplineComponent->GetLocationAtSplinePoint(SplineCount + 1, ESplineCoordinateSpace::Type::Local);
			const FVector EndTangent = SplineComponent->GetTangentAtSplinePoint(SplineCount + 1, ESplineCoordinateSpace::Type::Local);

			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
			SplineMesh->SetCollisionProfileName(TEXT("OverlapAll"));
			SplineMesh->OnComponentBeginOverlap.AddDynamic(this, &ACoverSpline::OnCoverBeginOverlap);
			SplineMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
			SplineMesh->SetStartAndEnd(StartPoint, StartTangent, EndPoint, EndTangent, true);


			UBoxComponent* CollisionBox = NewObject<UBoxComponent>(this, UBoxComponent::StaticClass());
			CollisionBox->SetWorldLocation(StartPoint);
			//	CollisionBox->SetWorldRotation(SplineComponent->GetRotationAtSplinePoint(SplineCount, ESplineCoordinateSpace::Type::Local));
			CollisionBox->SetCollisionProfileName(TEXT("OverlapAll"));
			CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ACoverSpline::OnCoverBeginOverlap);
			CollisionBox->AttachTo(SplineComponent);


			UArrowComponent* ArrowDirection = NewObject<UArrowComponent>(this, UArrowComponent::StaticClass());
			ArrowDirection->SetWorldLocation(StartPoint);
			//ArrowDirection->SetWorldRotation(SplineComponent->GetRotationAtSplinePoint(SplineCount, ESplineCoordinateSpace::Type::Local));
			ArrowDirection->SetCollisionProfileName(TEXT("OverlapAll"));
			ArrowDirection->OnComponentBeginOverlap.AddDynamic(this, &ACoverSpline::OnCoverBeginOverlap);
			ArrowDirection->AttachTo(SplineComponent);


			UStaticMesh* StaticMesh = DefaultMeshDetails->Mesh;
			UMaterialInterface* Material = nullptr;
			ESplineMeshAxis::Type ForwardAxis = DefaultMeshDetails->ForwardAxis;


			// start mesh
			if (StartMeshDetails && StartMeshDetails->Mesh && SplineCount == 0)
			{
				StaticMesh = StartMeshDetails->Mesh;
				ForwardAxis = StartMeshDetails->ForwardAxis;

				if (StartMeshDetails->DefaultMaterial)
				{
					Material = StartMeshDetails->DefaultMaterial;
				}
			}
			else if (EndMeshDetails && EndMeshDetails->Mesh && SplinePoints > 2 && SplineCount == (SplinePoints - 2))
			{
				// end mesh
				StaticMesh = EndMeshDetails->Mesh;
				ForwardAxis = EndMeshDetails->ForwardAxis;

				if (EndMeshDetails->DefaultMaterial)
				{
					Material = EndMeshDetails->DefaultMaterial;
				}
			}
			else
			{
				// default assignment - middle mesh
				if (DefaultMeshDetails->AlternativeMaterial && SplineCount > 0 && SplineCount % 2 == 0)
				{
					Material = DefaultMeshDetails->AlternativeMaterial;
				}
				else if (DefaultMeshDetails->DefaultMaterial)
				{
					Material = DefaultMeshDetails->DefaultMaterial;
				}
			}


			// update mesh details
			SplineMesh->SetStaticMesh(StaticMesh);
			SplineMesh->SetForwardAxis(ForwardAxis, true);
			SplineMesh->SetMaterial(0, Material);


			SplineRotation = FRotator(0.0F, 180.0F, 0.0F);

			// initialize the object
			SplineMesh->RegisterComponentWithWorld(GetWorld());

			SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			SplineMesh->SetMobility(EComponentMobility::Movable);

			SplineMesh->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepRelativeTransform);

		}
	}
}

void ACoverSpline::BeginPlay()
{
	Super::BeginPlay();


}

void ACoverSpline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACoverSpline::FollowSpline(APawn* Character, float InputVal)
{

	auto distance = GetSplineComponent()->GetSplineLength();
	FVector splinePointLocation = GetSplineComponent()->GetLocationAtDistanceAlongSpline(distance, ESplineCoordinateSpace::World);
	FRotator splinePointRotation = GetSplineComponent()->GetRotationAtDistanceAlongSpline(distance, ESplineCoordinateSpace::World);


	FVector Direction = GetSplineComponent()->FindTangentClosestToWorldLocation(Character->GetActorLocation(), ESplineCoordinateSpace::World);
	Character->AddActorWorldRotation(splinePointRotation);
	Character->AddMovementInput(Direction, InputVal);
}

int ACoverSpline::GetTotalSplinePoints()
{
	if (SplineComponent)
	{
		return SplineComponent->GetNumberOfSplinePoints();
	}

	return 0;
}

void ACoverSpline::OnCoverBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
	}
}

void ACoverSpline::set_rotation_at_spline_point(USplineComponent* target, const int32 point_index, const FRotator rotation)
{
	FInterpCurveQuat& SplineRotInfo = target->GetSplinePointsRotation(); //get the array of rotation data in the spline component

	FInterpCurvePoint<FQuat>& EditedRotPoint = SplineRotInfo.Points[point_index]; //get the point to edit

	FQuat NewRot = rotation.Quaternion(); //convert the given rotation into a quaternion

	EditedRotPoint.OutVal = NewRot; //set the new rotation of the selected point
}

